#include "SaveDataToDB.h"

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

bool SaveDataToDB::Init(QString* err)
{
    m_islogevery_data = false;
    bool res = Init(rtdb_filename, err) && Init(db_filename, err);
    if(res)
    {
        Start();
    }
    return res;
}

bool SaveDataToDB::Init( QString filelname, QString* err /*= nullptr*/ )
{
    {
        //��ʼ�����ݿ�
        QFile f(db_path+filelname);
        if(!f.exists())
        {//û�д�ʱ���Զ������ļ�����˴�ʱ����Ҫִ�д�����䱣֤�������ݱ��Ѵ���
            QDir d ;
            if(!d.exists(db_path))
            {
                bool t = d.mkpath(db_path);
            }
            db.setDatabaseName(db_path+db_filename);
            if(!db.open())
            {
                SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filelname).arg(db.lastError().text()))) ;
                return false;
            }
            //�½����ļ� ִ��һ�����ݱ�Ĵ������
            QSqlQuery query(db);
            bool res = true;
            do{
                res = query.exec(create_tb_Main_SQL);
                if(!res)
                {
                    break;
                }
                res = query.exec(create_tb_Mold_SQL);
                if(!res)
                {
                    break;
                }
                res = query.exec(create_tb_Sensor_SQL);
                if(!res)
                {
                    break;
                }
                res = query.exec(create_tb_SensorAdd_SQL);
                if(!res)
                {
                    break;
                }
            } while (false);
            if(!res)
            {
                SAFE_SET(err, QString(QObject::tr("Initial database file:%1 failure,the erro is %2").arg(filelname).arg(query.lastError().text()))) ;
                db.close();
                return false;
            }
        }
        else{//�����
            db.setDatabaseName(db_path+filelname);
            if(!db.open())
            {
                SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filelname).arg(db.lastError().text()))) ;
                return false;
            }
        }
        db.close();
    }

    //QSqlDatabase::removeDatabase(filelname);
    return true;
}

bool SaveDataToDB::SaveData(const XmlData& data, QString* err)
{
    ParserStrategyFile(true);
    {
        QMutexLocker lk(rtdb_mutex.data());
        bool res = true;

        //�����ݿ�
        db.setDatabaseName(db_path+rtdb_filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+db.lastError().text())) ;
            return false;
        }
        if(!db.transaction())
        {//��������ʧ��
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+db.lastError().text())) ;
            db.close();
            return false;
        }

        do
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
                if(mold.inspected == 0)
                {
                    continue;
                }
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
                    if(sensor.rejects == 0)
                    {
                        continue;
                    }
                    if(!checkSensorIDIsShouldSave(sensor.id))
                    {//���ݲ��ԣ�ȱ�����ݲ��ñ���
                        continue;
                    }
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

            db.commit();
            db.close();
        }while(false); 

        if(!res)
        {
            db.rollback();
            db.close();
            return false;
        }
    }

    //QSqlDatabase::removeDatabase("record_db");
    return true;
}

void SaveDataToDB::ParserStrategyFile(bool first)
{
    if(first && !QFile::exists(strategyfile))
    {
        strategy_mode = true;
        sensorids.push_back(10);    //��ʯ
        sensorids.push_back(34);    //С��ʯ

        SaveStrategyData();
        lastmodify = QDateTime::currentDateTime();
        return;
    }

    QDomDocument doc("strategyfile");
    QFile file(strategyfile);
    if (!file.open(QIODevice::ReadOnly))
    {//�ļ���ʧ�ܣ�����ǵ�һ�� ������ΪĬ������ ���򲻹�
        if(first)
        {
            strategy_mode = true;
            sensorids.push_back(10);    //��ʯ
            sensorids.push_back(34);    //С��ʯ

            SaveStrategyData();
            lastmodify = QDateTime::currentDateTime();
            return;
        }
        return;
    }

    QFileInfo info(file);
    QDateTime t = info.lastModified();
    if(!first)
    {
        if (t.secsTo(lastmodify)>0)
        {//�ļ��޸Ķ�
            return;
        }
    }
    lastmodify = t;
    //����ΪXML�ļ���׼��ʽ
    ///*<strategy>
    //    <!--����ģʽ true��ʾ������ģʽ ������ʾ������-->
    //    <mode>true</mode>
    //    <sensor>
    //    <id>10</id>
    //    <id>34</id>
    //    </sensor>
    //    <islogall>true</islogall>
    //    </strategy>*/

    //���������ļ�
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    // ����XML�ļ�
    QDomElement root = doc.documentElement();
    if(root.tagName() != "strategy")
    {
        //"XML data the top tag isn't strategy,the xml data has incorrect format"
        return ;
    }
    if(!root.hasChildNodes())
    {//û���ӽڵ�����
        //"Paser XML data,Root Node, No Child";
        return ;
    }
    QDomElement mode = root.firstChildElement("mode");
    if(mode.isNull())
    {//û�д˽ڵ�
        //"Paser XML data,No Called mode child Node";
        return ;
    }
    strategy_mode = (mode.text() == "true")?true:false;
    QDomElement logall = root.firstChildElement("islogall");
    if(logall.isNull())
    {//û�д˽ڵ�
        m_islogevery_data = false;
    }
    else
    {
        m_islogevery_data = (logall.text() == "true")?true:false;
    }

    QDomElement sensor = root.firstChildElement("sensor");
    if(!sensor.hasChildNodes())
    {
        return;
    }
    QDomNodeList lst = sensor.childNodes();
    for(int i = 0; i < lst.length();i++)
    {
        sensorids.push_back(lst.at(i).toElement().text().toInt());
    }
}

bool SaveDataToDB::SaveStrategyData( QString* err /*= nullptr*/ )
{
    QFile file(strategyfile);
    if(!file.open(QIODevice::WriteOnly))
    {
        SAFE_SET(err, QObject::tr("write strategy data failure: open file failure"));
        return false; //�½��ļ���ʧ��
    }

    QDomDocument doc("strategydoc");

    QDomComment cm = doc.createComment(QObject::tr("mode: true for whitelist, the other for blacklist, the id in sensor is the id affected by mode"));
    doc.appendChild(cm);

    QDomElement root = doc.createElement("strategy");
    QDomElement mode = doc.createElement("mode");
    mode.appendChild(doc.createTextNode( strategy_mode?QObject::tr("true"):QObject::tr("false")));
    QDomElement sensor = doc.createElement("sensor");
    foreach(int id, sensorids)
    {
        QDomElement n_id = doc.createElement("id");
        n_id.appendChild(doc.createTextNode(QString::number(id)));
        sensor.appendChild(n_id);
    }
    root.appendChild(mode);
    root.appendChild(sensor);

    doc.appendChild(root);

    QTextStream stream(&file);
    stream.setCodec("UTF-8"); //ʹ��utf-8��ʽ

    doc.save(stream, 4, QDomNode::EncodingFromTextStream);
    file.close();
    return true;
}

bool SaveDataToDB::checkSensorIDIsShouldSave( int id )
{
    if(strategy_mode)
    {//������
        foreach(int i, sensorids)
        {
            if(i == id)
            {
                return true;
            }
        }
        return false;
    }
    else
    {//������
        foreach(int i, sensorids)
        {
            if(i == id)
            {
                return false;
            }
        }
        return true;
    }
}

void SaveDataToDB::Start()
{
    thd->start();
    emit sig_start();
}

void SaveDataToDB::Stop()
{
    work->runflg = false;
    thd->wait(1000);
    thd->terminate();
    ELOGD("SaveDataToDB::Stop()");
}

SaveDataToDB::SaveDataToDB( QObject* parent /*= nullptr*/ )
    : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE", "SaveDataToDB");
    db_mutex = QSharedPointer<QMutex>(new QMutex);
    rtdb_mutex = QSharedPointer<QMutex>(new QMutex);

    work = new GenerateRecord(rtdb_mutex, db_mutex);
    thd = new QThread(this);

    work->moveToThread(thd);
    connect(this, SIGNAL(sig_start()), work, SLOT(work()));
    connect(thd, SIGNAL(finished()), work, SLOT(deleteLater()));
}

SaveDataToDB::~SaveDataToDB()
{
    Stop();
}

bool SaveDataToDB::GetRecordByTime( QDateTime st, QDateTime end, Record& data )
{
    //��ԭʼ��¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
    QString  filename = db_filename;
    {
        QMutexLocker lk(db_mutex.data());
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            QString err = QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()));
            ELOGE(err.toLocal8Bit().constData() ) ;
            return false;
        }
        {//Ϊ�˹ر�����ʱ û��queryʹ��
            //��ѯ����
            QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
                                  FROM %1 WHERE TimeStart >= '%2' and TimeStart <= '%3';")
                                  .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(end.toString("yyyy-MM-dd hh:mm:ss"));
            ELOGD(qPrintable(sql));
            QSqlQuery query(db);
            if(!query.exec(sql) )
            {
                QString err =  QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text());
                ELOGE(err.toLocal8Bit().constData() ) ;
                db.close();
                return false;
            }
            if(query.next())
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
                    QSqlQuery mquery(db);
                    if(!mquery.exec(sql))
                    {//��ѯʧ��
                        QString err =  QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text());
                        ELOGE(err.toLocal8Bit().constData() ) ;
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

                        {//��ѯsensor��Ϣ
                            sql = QString("SELECT ID, SensorID, Rejects, Defects FROM %1 WHERE TMoldRowID=%2")   
                                .arg(tb_Sensor).arg(moldrowid);
                            QSqlQuery squery(db);
                            if(!squery.exec(sql))
                            {
                                QString err =  QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text());
                                ELOGE(err.toLocal8Bit().constData() ) ;
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

                                {//��ѯsensoradd��Ϣ
                                    sql = QString("SELECT ID, CounterID, Nb FROM %1 WHERE TSensorRowID=%2")   
                                        .arg(tb_SensorAdd).arg(sensorrowid);
                                    QSqlQuery aquery(db);
                                    if(!aquery.exec(sql))
                                    {
                                        QString err =  QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text());
                                        ELOGE(err.toLocal8Bit().constData() ) ;
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
                data = re;
            }
            else
            {
                Record r;
                data = r;
            }
        }
        db.close();
    }

    return true;
}

bool SaveDataToDB::GetLastestRecord( Record& data, QString* err )
{
    //��ԭʼ��¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
    QString  filename = db_filename;
    {
        QMutexLocker lk(db_mutex.data());
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
            return false;
        }
        {//Ϊ�˹ر�����ʱ û��queryʹ��
            //��ѯ����
            QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
                                  FROM %1 ORDER BY TimeEnd desc limit 1;").arg(tb_Main);
            QSqlQuery query(db);
            if(!query.exec(sql) )
            {
                SAFE_SET(err, QString(QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                db.close();
                return false;
            }
            if(query.next())
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
                data = re;
            }
            else
            {
                Record t;
                data = t;
            }
        }
        db.close();
    }

    return true;
}

ETimeInterval SaveDataToDB::GetTimeInterval()
{
    return work->eti;
}

void SaveDataToDB::SetTimeInterval( ETimeInterval timeinterval )
{
    work->eti = timeinterval;
}

bool SaveDataToDB::GetAllDate( QList<QDate>& lst )
{
    lst.clear();
    QString  filename = db_filename;
    {
        QMutexLocker lk(db_mutex.data());
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
             QString err = QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()) ;
             ELOGE(err.toLocal8Bit().constData());
            return false;
        }
        {//Ϊ�˹ر�����ʱ û��queryʹ��
            //��ѯ����
            QString sql = QString("SELECT distinct (date(TimeStart)) as date From %1").arg(tb_Main);
            QSqlQuery query(db);
            if(!query.exec(sql) )
            {
                QString err = QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()) ;
                ELOGE(err.toLocal8Bit().constData());

                db.close();
                return false;
            }
            while(query.next())
            {
                QDate t = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
                lst.push_back(t);
            }
        }
        db.close();
    }

    return true;
}

bool SaveDataToDB::GetRecordListByDay( QDate date, QList<QTime>& stlst, QList<QTime>& etlst )
{
    QString tt = date.toString();

    QDateTime st(date, QTime(0,0,0));
    QDateTime et = st.addDays(1);
    
    stlst.clear();
    etlst.clear();
    //�Ӽ�¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
    QString  filename = db_filename;
    {
        QMutexLocker lk(db_mutex.data());
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            QString err = QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()) ;
            ELOGE(err.toLocal8Bit().constData());
            return false;
        }
        {//Ϊ�˹ر�����ʱ û��queryʹ��
            //��ѯ����
            QString sql = QString("SELECT TimeStart,TimeEnd From %1 WHERE TimeStart >= '%2' and TimeEnd <= '%3';")
                .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
            ELOGD(qPrintable(sql));
            QSqlQuery query(db);
            if(!query.exec(sql) )
            {
                QString err = QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()) ;
                ELOGE(err.toLocal8Bit().constData());

                db.close();
                return false;
            }
            while(query.next())
            {
                QTime t = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss").time();
                stlst.push_back(t);
                t = QDateTime::fromString(query.value(1).toString(), "yyyy-MM-dd hh:mm:ss").time();
                etlst.push_back(t);
            }
        }
        db.close();
    }

    return true;
}
