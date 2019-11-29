#include "GenerateRecord.h"

#include <QString>
#include <QObject>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>

#include <QEventLoop>
#include <QTimer>

#include <QDomDocument>
#include "logger.h"

#include "DBOperDef.hpp"

#include <QMutexLocker>

bool GenerateRecord::GetLastestRecord(QDateTime& t, QString* err)
{//�豣֤���ݿ��ļ��Ѿ�����,�ҽṹ��ȷ
    /*************************************************************************
    * ��ȡ��ʼ��ʱ���
    * 1 �Ӽ�¼��� ��ȡ���¼�¼�Ľ�ֹʱ�� 
    * 2 ���û�м�¼,���ȡԭʼ��¼�ĵ�һ�����ݵ���ʼʱ�� 
    *************************************************************************/
    QString filename = db_filename;
    {   //1 �Ӽ�¼��� ��ȡ���¼�¼�Ľ�ֹʱ��
        QMutexLocker lk(db_mutex.data());
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
            return false;
        }
        {//Ϊ�˹ر�����ʱ û��queryʹ��
            QString sql = QString("SELECT max(TimeEnd) as result FROM %1 ;").arg(tb_Main);
            QSqlQuery query(sql, db);
            if(!query.exec() || !query.next())
            {
                SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                db.close();
                return false;
            }
            else
            {//��ȡ���
                if(!query.value(0).isNull())
                {//û������,���ΪNULL
                    t = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss");
                    //ELOGD("GetLastT DT:%s", qPrintable(t.toString()));
                    db.close();
                    return true;
                }
            }
        }
        db.close();
        //
    }
    {
        QMutexLocker lk(rtdb_mutex.data());
        filename = rtdb_filename;
        //1 �Ӽ�¼��� ��ȡ���¼�¼�Ľ�ֹʱ��
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
            return false;
        }
        {//Ϊ�˹ر�����ʱ û��queryʹ��
            QString sql = QString("SELECT min(TimeStart) as result FROM %1;").arg(tb_Main);
            QSqlQuery query(sql, db);
            if(!query.exec() || !query.next())
            {
                SAFE_SET(err, QString(QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                db.close();
                return false;
            }
            else
            {//��ȡ���
                if(!query.value(0).isNull())
                {//û������,���ΪNULL
                    t = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss");
                    //QString st = t.toString();
                    //ELOGD("GetLastT RT_T:%s", qPrintable(t.toString()));
                    db.close();
                    return true;
                }
            }
        }
    }
    //���ᵽ��ķ�֧
    ELOGE("%s can't access branch", __FUNCTION__);
    return false;
}

bool GenerateRecord::QueryRecordByTime(QDateTime st, QDateTime et,QList<Record>& lst, QString* err)
{
    //��ԭʼ��¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
    QString  filename = rtdb_filename;
    QMutexLocker lk(db_mutex.data());
    {
        QMutexLocker lk(rtdb_mutex.data());
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
            return false;
        }
        {//Ϊ�˹ر�����ʱ û��queryʹ��
            //��ѯ����
            QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
                                  FROM %1 WHERE TimeStart >= '%2' and TimeStart < '%3';")
                                  .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
            //ELOGD(qPrintable(sql));
            QSqlQuery query(db);
            if(!query.exec(sql) )
            {
                SAFE_SET(err, QString(QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                db.close();
                return false;
            }
            while(query.next())
            {
                Record re;
                int mainrowid    = query.value(0).toInt();
                re.id            = query.value(1).toString();
                re.inspected     = query.value(2).toInt();
                re.rejects       = query.value(3).toInt();
                re.defects       = query.value(4).toInt();
                re.autorejects   = query.value(5).toInt();
                re.dt_start      = QDateTime::fromString(query.value(6).toString(), "yyyy-MM-dd hh:mm:ss"); 
                re.dt_end        = QDateTime::fromString(query.value(7).toString(), "yyyy-MM-dd hh:mm:ss"); 
                if(re.inspected > 0)
                {//������������0����������Ϣ
                    sql = QString("SELECT ID, MoldID, Inspected, Rejects, Defects, Autoreject FROM %1 WHERE TMainRowID=%2;")
                        .arg(tb_Mold).arg(mainrowid);
                    //ELOGD(qPrintable(sql));
                    QSqlQuery mquery(db);
                    if(!mquery.exec(sql))
                    {//��ѯʧ��
                        SAFE_SET(err, QString(QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                        db.close();
                        return false;
                    }
                    while(mquery.next())
                    {
                        MoldInfo mold;
                        int moldrowid      = mquery.value(0).toInt();
                        mold.id            = mquery.value(1).toInt();
                        mold.inspected     = mquery.value(2).toInt();
                        mold.rejects       = mquery.value(3).toInt();
                        mold.defects       = mquery.value(4).toInt();
                        mold.autorejects   = mquery.value(5).toInt();
                        if(mold.inspected <= 0)
                        {
                            continue;
                        }
                        {//��ѯsensor��Ϣ
                            sql = QString("SELECT ID, SensorID, Rejects, Defects FROM %1 WHERE TMoldRowID=%2")   
                                .arg(tb_Sensor).arg(moldrowid);
                            //ELOGD(qPrintable(sql));
                            QSqlQuery squery(db);
                            if(!squery.exec(sql))
                            {
                                SAFE_SET(err, QString(QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                                db.close();
                                return false;
                            }
                            while(squery.next())
                            {
                                SensorInfo sensor;
                                int sensorrowid      = squery.value(0).toInt();
                                sensor.id            = squery.value(1).toInt();
                                sensor.rejects       = squery.value(2).toInt();
                                sensor.defects       = squery.value(3).toInt();
                                if(sensor.rejects <= 0)
                                {
                                    continue;
                                }
                                {//��ѯsensoradd��Ϣ
                                    
                                    sql = QString("SELECT ID, CounterID, Nb FROM %1 WHERE TSensorRowID=%2")   
                                        .arg(tb_SensorAdd).arg(sensorrowid);
                                    //ELOGD(qPrintable(sql));
                                    QSqlQuery aquery(db);
                                    if(!aquery.exec(sql))
                                    {
                                        SAFE_SET(err, QString(QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                                        db.close();
                                        return false;
                                    }
                                    while(aquery.next())
                                    {
                                        SensorAddingInfo addi;
                                        addi.counter_id    = aquery.value(0).toInt();
                                        addi.nb            = aquery.value(1).toString();

                                        sensor.addinginfo.push_back(addi);
                                    }
                                }
                                
                                mold.sensorinfo.push_back(sensor);
                            }
                        }
                        re.moldinfo.push_back(mold);
                    }
                }
                lst.push_back(re);
            }
        }
        db.close();
    }

    return true;
}

/**
* @brief  :  CalcGenerateRecordTime ���ص�����ʼʱ��
*
* @param  :  QDateTime start
* @param  :  ETimeInterval eti
* @param  :  QList<QDateTime> & lst
* @return :  void
* @retval :
*/
void GenerateRecord::CalcGenerateRecordTime(QDateTime start, ETimeInterval eti, QList<QDateTime>& lst)
{
    int ti = ETimeInterval2Min(eti);
    QDateTime end = QDateTime::currentDateTime();
    //�����һ��ʱ��������ʼʱ��
    QDateTime t(start.date(), QTime(0,0));
    //���賿12�㿪ʼ������ʼʱ�����ڵ�����,
    int passd = t.secsTo(start)/60/ti;
    QDateTime st = t;
    st = st.addSecs(passd*ti*60);
    QStringList tslst;

    while(st.secsTo(end) > ti*60 )
    {//������Ҫ���ɵļ�¼����
        tslst.push_back(st.toString());

        lst.push_back(st);
        st = st.addSecs(ti*60);
    }

    if(!lst.isEmpty())
    {
        //���һ���ǽ�ֹʱ��
        lst.push_back(st);
    }
}

bool GenerateRecord::StatisticsRecord(QList<QDateTime>& tlst, QList<Record>& lst, QList<Record>& newlst, QString* err)
{//�����������ݲ���Ϊ��
    if(tlst.size() < 2 || lst.isEmpty())
    {//ʱ����Ϊ�� ����Ԫ����Ϊ��
        //����ʱ��ֽ������������(��ʼ+����),����ж�С��2
        SAFE_SET(err, QObject::tr("the data isn't enough"));
        return false;
    }
    /**************************************************
    * 1 ͬʱ����ʱ��ֽ������ݽڵ�
    * ע:Ĭ�������������(ʱ������ ͳ�Ƽ�¼ʱ������),
    ***************************************************/

    int j = 0;
    for(int i = 0; i < tlst.size()-1; i++)
    {//����ʱ��ֽ��
        Record r;
        r.dt_start = tlst[i];
        r.dt_end = tlst[i+1];
        //ͳ�Ƹ�ʱ��ε�����
        for(; j < lst.size();j++)
        {
            QString t1s = r.dt_start.toString();
            QString t1e = r.dt_end.toString();
            QString t2s = lst[j].dt_start.toString();
            QString t2e = lst[j].dt_end.toString();
            if(lst[j].dt_end <= r.dt_end && lst[j].dt_end >= r.dt_start)
            {//ԭ��ֻ���ж���ֹʱ���ڱ�ѡʱ���֮ǰ
                r.id = lst[j].id;
                r.MergeData(lst[j]);
            }
            else if(lst[j].dt_end > r.dt_end)
            {//������һ��ʱ��ε�����,���˳�,��֯��һ��ͳ�Ƽ�¼
                break;
            }
        }
        newlst.push_back(r);
        if(j == lst.size())
        {
            break;
        }
    }
    return true;
}

bool GenerateRecord::GenerateRecordToDB(QList<Record>& newlst, QString* err)
{
    QString filename = db_filename;
    {
        bool res = true;
        QMutexLocker lk(db_mutex.data());

        //�����ݿ�
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
            return false;
        }
        if(!db.transaction())
        {//��������ʧ��
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+db.lastError().text())) ;
            db.close();
            return false;
        }

        foreach(Record data, newlst)
        {//�����������
            //�½����ļ� ִ��һ�����ݱ�Ĵ������
            QSqlQuery query(db);

            //���ݲ��Ծ������������
            QString sql = QString("INSERT INTO %1(MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd) \
                                  Values('%2', %3, %4, %5, %6, '%7', '%8');").arg(tb_Main).arg(data.id).arg(data.inspected).arg(data.rejects).arg(data.defects)
                                  .arg(data.autorejects).arg(data.dt_start.toString("yyyy-MM-dd hh:mm:ss")).arg(data.dt_end.toString("yyyy-MM-dd hh:mm:ss"));

            //�����������ݣ�����ѯ�����Ӧ��IDֵ
            if(!query.exec(sql))
            {
                SAFE_SET(err, QString(QObject::tr("Save Machine Data Error, the Error is ")+query.lastError().text())) ;
                res = false;
                break;
            }
            //��ѯ�����Ӧ��IDֵ��������IDֵ���
            if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Main)))
            {
                SAFE_SET(err, QString(QObject::tr("Query Main Data ID Error: ")+query.lastError().text())) ;
                res = false;
                break;
            }
            //�����ܳ��ֵ����� 
            if(!query.next() || query.isNull(0))
            {
                SAFE_SET(err, QString(QObject::tr("query in a exception, no main data")+query.lastError().text())) ;
                res = false;
                break;
            }
            int mainrowid = query.value(0).toInt();

            foreach(MoldInfo mold, data.moldinfo)
            {
                //ѭ������ÿ��ģ����Ϣ���ݣ�����ѯÿ��ģ���Ӧ��IDֵ
                sql = QString("INSERT INTO %1(TMainRowID, MoldID, Inspected, Rejects, Defects, Autoreject) \
                              VALUES(%2, %3, %4, %5, %6, %7);").arg(tb_Mold).arg(mainrowid).arg(mold.id)
                              .arg(mold.inspected).arg(mold.rejects).arg(mold.defects).arg(mold.autorejects);
                if(!query.exec(sql))
                {
                    SAFE_SET(err, QString(QObject::tr("Save Mold Data Error, the Error is ")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                //��ѯģ����Ӧ��IDֵ��������IDֵ���
                if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Mold)))
                {
                    SAFE_SET(err, QString(QObject::tr("Query Mold Data ID Error: ")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                //�����ܳ��ֵ����� 
                if(!query.next() || query.isNull(0))
                {
                    SAFE_SET(err, QString(QObject::tr("query in a exception, no mold data")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                int moldrowid = query.value(0).toInt();

                foreach(SensorInfo sensor, mold.sensorinfo)
                {
                    //ѭ�����浥��ģ�������ȱ����Ϣ���ݣ�����¼ÿ��ȱ�����ݵ�ID
                    sql = QString("INSERT INTO %1(TMoldRowID, SensorID, Rejects, Defects) \
                                  VALUES(%2, %3, %4, %5);").arg(tb_Sensor).arg(moldrowid).arg(sensor.id).arg(sensor.rejects)
                                  .arg(sensor.defects);
                    if (!query.exec(sql))
                    {
                        SAFE_SET(err, QString(QObject::tr("Save Sensor Data Error, the Error is ")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    //��ѯģ����Ӧ��IDֵ��������IDֵ���
                    if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Sensor)))
                    {
                        SAFE_SET(err, QString(QObject::tr("Query Sensor Data ID Error: ")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    //�����ܳ��ֵ����� 
                    if(!query.next() || query.isNull(0))
                    {
                        SAFE_SET(err, QString(QObject::tr("query in a exception, no sensor data")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    int sensorrowid = query.value(0).toInt();

                    foreach(SensorAddingInfo adding, sensor.addinginfo)
                    {
                        //ѭ�����浥��ȱ�ݵĸ�����Ϣ����
                        sql = QString("INSERT INTO %1(TSensorRowID, CounterID, Nb) VALUES(%2,%3,%4);")
                            .arg(tb_SensorAdd).arg(sensorrowid).arg(adding.counter_id).arg(adding.nb);
                        if(!query.exec(sql))
                        {
                            SAFE_SET(err, QString(QObject::tr("Save SensorAdd Data Error, the Error is ")+query.lastError().text())) ;
                            res = false;
                            break;
                        }
                    }
                    if(!res)
                    {
                        break;
                    }
                }
                if(!res)
                {
                    break;
                }
            }
            if(!res)
            {
                break;
            }
        } 

        if(!res)
        {
            db.rollback();
        }
        else
        {
            db.commit();
        }
        db.close();
        return res;
    }

    //QSqlDatabase::removeDatabase(filename);
    return false;
}

void GenerateRecord::DeleteOutdatedData(int outdatedays)
{
    //��ǰʱ��
    QString t = QDateTime::currentDateTime().addDays(outdatedays*-1).toString("yyyy-MM-dd hh:mm:ss");
    //��������� ���ֱ��ɾ���������� ����ɾ���������
    {
        QMutexLocker lk(rtdb_mutex.data());
        QString filename = rtdb_filename;
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", filename);
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            ELOGD(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()).toLocal8Bit().constData());
            return ;
        }
        {
            QString sql = QString("DELETE FROM %1 WHERE TimeEnd < '%2';").arg(tb_Main).arg(t);
            QSqlQuery query(db);
            if(!query.exec(sql))
            {
                ELOGD(QObject::tr("Delete database file:%1  data failure,the erro is %2").arg(filename).arg(db.lastError().text()).toLocal8Bit().constData());
            }

        }
        db.close();
    }
}

GenerateRecord::GenerateRecord(QSharedPointer<QMutex> rt_mx,QSharedPointer<QMutex> mx, QObject *parent /*= nullptr*/ )
    : QObject(parent)
    , runflg(true)
{
    eti = ETI_30_Min;
    //Ĭ��30�����
    outdatedays = 30;

    db_mutex = mx;
    rtdb_mutex = rt_mx;
    db = QSqlDatabase::addDatabase("QSQLITE", "GenerateRecord");
}

void GenerateRecord::work()
{
    /***************************************************************************************************
    * ����ʵ��:
    *  ��ʱɾ������ԭʼ��¼
    *
    * 1 ����¼���ݿ������,�õ� ���µ���־��ֹʱ�� QDateTime t �п���û��
    * 2 ����t (û��t,����ʵʱ��¼�ĵ�һ������ʼʱ��Ϊ׼),������Ҫ��ӵļ�¼���,
    * 3 ���ʵʱ�������ݿ�,�õ�������t֮�������,���û��t,���ȡ����
    * 4 ����ʱ���� �� ԭʼ����,ͳ�Ƶõ���¼����
    * 5 ����ͳ�Ƽ�¼����,����ÿһ����¼,�ŵ���¼���ݿ���
    *
    ****************************************************************************************************/
    bool res = false;

    QDateTime dellast = QDateTime::currentDateTime();
    dellast = dellast.addDays(-2);
    QString err;
    while (runflg) 
    {//���ɼ�¼

        do 
        {
            //��ʱɾ����¼ ÿ��ִ��һ��,
            int d = dellast.daysTo(QDateTime::currentDateTime());
            if(dellast.daysTo(QDateTime::currentDateTime()) >= 1)
            {
                DeleteOutdatedData(outdatedays);
                dellast = QDateTime::currentDateTime();
            }
            if(!runflg){
                break;
            }
            QDateTime t;
            //1 ����¼���ݿ������,�õ� ���µ���־��ֹʱ�� QDateTime t,�����ȡʧ��,���ԭʼ��¼���ȡ��һ����¼����ʼʱ��
            res = GetLastestRecord(t, &err);
            if(!res)
            {
                ELOGD(qPrintable(err));
            }
            //ELOGD("GetLastT: %s", qPrintable(t.toString()));

            //2 ���ݽ�ֹʱ�� ��ʱ���� ������Ҫ���ɼ�¼��ʱ����
            QList<QDateTime> timelst;
            CalcGenerateRecordTime(t, eti, timelst);
            if(timelst.isEmpty())
            {
                break;
            }
            if(!runflg){
                break;
            }
            QDateTime et = timelst.last();

            //3 ���ʱ���ʱ���� �鿴�Ƿ������¼�¼��Ҫ����
            QList<Record> record_lst;
            res = QueryRecordByTime(t, et, record_lst, &err);
            if(record_lst.isEmpty())
            {
                break;
            }

            if(!runflg){
                break;
            }

            //4 �������������б�ͳ��ÿ�����������
            QList<Record> statistic_record_lst;
            res = StatisticsRecord(timelst, record_lst, statistic_record_lst, &err);
            if(!runflg){
                break;
            }
            //5 ���ɼ�¼
            res = GenerateRecordToDB(statistic_record_lst, &err);

        } while (false);

       
        int cnt = 5000;
        while(cnt > 0)
        {
            int single = 200;
            //���ɼ�¼�ܺ�ʱ,����ʵʱ��Ҫ�󲻸�,��˿�������Ϊ5 sec
            QEventLoop eventloop;
            QTimer::singleShot(single, &eventloop, SLOT(quit()));
            eventloop.exec();
            if(!runflg){
                break;
            }
            cnt -= single;
        }
        if(!runflg){
            break;
        }
    }
    emit resultReady(tr("GenerateRecord::work exit"));

    ELOGD("GenerateRecord Exit");
}

GenerateRecord::~GenerateRecord()
{
    runflg = false;
}
