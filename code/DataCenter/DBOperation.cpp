#include "DBOperation.h"
#include "DataDef.hpp"
#include "DBOperDef.hpp"

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

#include <QMutexLocker>

bool DBOperation::SaveData(const XmlData& data, QString* err)
{
    try
    {
        QMutexLocker lk(rtdb_mutex.data());
        return SaveRecord(EOperDB_RuntimeDB, data, err);
    }
    catch (std::exception& e)
    {
        ELOGE("Save Data To Database exception:%s", e.what());
    }
    catch (...)
    {
    }
    return false;

}

DBOperation::DBOperation( QObject* parent /*= nullptr*/ )
    : DBOperationB(parent)
{

}

DBOperation::~DBOperation()
{

}

bool DBOperation::GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data )
{
    try
    {
        EOperDatabase dbtype;
        if(!GetDBOperType((ERecordType)type, dbtype))
        {
            return false;
        }

        //�Ӽ�¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
        QList<Record> relst;
        QString err;
        if(!QueryRecordByTime(dbtype, st, end, relst, &err))
        {
            return false;
        }
        if(relst.isEmpty())
        {
            return false;
        }
        data = relst.first();
        return true;
    }
    catch (std::exception& e)
    {
        ELOGE("GetRecordByTime ST:%s ET:%s exception:%s ", st.toString().toStdString().c_str(), end.toString().toStdString().c_str(), e.what());
    }
    catch (...)
    {
    }
    return false;

}

bool DBOperation::GetLastestRecord(int type, Record& data, QString* err )
{
    try
    {
        bool res = true;

        {
            QSharedPointer<QSqlDatabase> _db;
            QSharedPointer<QMutex> _db_mutex;
            if(!GetRecordDBParams((ERecordType)type, _db, _db_mutex))
            {
                return false;
            }

            QMutexLocker lk(_db_mutex.data());
            if(!_db->open())
            {
                SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(_db->lastError().text()))) ;
                ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
                return false;
            }
            bool res = true;
            do 
            {//Ϊ�˹ر�����ʱ û��queryʹ�� //��ѯmain information
                QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
                                      FROM %1 ORDER BY TimeEnd desc limit 1;").arg(tb_Main);
                if(type == (int)ERT_Shift)
                {
                    sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd, Date, Shift \
                                  FROM %1 ORDER BY TimeEnd desc limit 1;").arg(tb_Main);
                }
                //��ѯ����

                QSqlQuery query(*_db);
                if(!query.exec(sql) )
                {
                    SAFE_SET(err, QString(QObject::tr("Query RTRecord database failure,the erro is %1").arg(query.lastError().text()))) ;
                    res = false;
                    break;
                }
                QMap<int, Record> mainrow_record;
                QMap<int, QList<MoldInfo> > mainrow_moldlst;
                QMap<int, QList<SensorInfo> > moldrow_sensorlst;
                QMap<int, QMap<QString, int> > sensorrow_adding;

                QSet<int> mainrowidlst, moldrowidlst, sensorrowidlst;

                if(query.next())
                {
                    data.mainrowid    = query.value(0).toInt();
                    data.id            = query.value(1).toString();
                    data.inspected     = query.value(2).toInt();
                    data.rejects       = query.value(3).toInt();
                    data.defects       = query.value(4).toInt();
                    data.autorejects   = query.value(5).toInt();
                    data.dt_start      = QDateTime::fromString(query.value(6).toString(), "yyyy-MM-dd hh:mm:ss"); 
                    data.dt_end        = QDateTime::fromString(query.value(7).toString(), "yyyy-MM-dd hh:mm:ss"); 
                    if(type == ERT_Shift)
                    {
                        data.date = QDate::fromString(query.value(8).toString(), "yyyy-MM-dd");
                        data.shift = query.value(9).toInt();
                    }
                    if(data.inspected <= 0)
                    {//������������0����������Ϣ
                        continue;
                    }
                    mainrowidlst.insert(data.mainrowid);
                }

                if(!QueryMoldInfoByMainID(_db, mainrowidlst, mainrow_moldlst, moldrowidlst, err))
                {
                    res = false;
                    break;
                }
                if(!QuerySensorInfoByMoldID(_db, moldrowidlst, moldrow_sensorlst, sensorrowidlst, err))
                {
                    res = false;
                    break;
                }
                if(!QuerySensorAddingInfoBySensorID(_db, sensorrowidlst, sensorrow_adding, err))
                {
                    res = false;
                    break;
                }
                if(mainrow_moldlst.contains(data.mainrowid))
                {
                    data.moldinfo = mainrow_moldlst[data.mainrowid];
                    QList<MoldInfo>& m = data.moldinfo;
                    for(int j = 0; j < m.size(); j++)
                    {
                        if(moldrow_sensorlst.contains(m[j].moldrowid))
                        {
                            m[j].sensorinfo = moldrow_sensorlst[m[j].moldrowid];
                            QList<SensorInfo>& slst = m[j].sensorinfo;
                            for(int k = 0; k < slst.size(); k++)
                            {
                                if(sensorrow_adding.contains(slst[k].sensorrowid))
                                {
                                    slst[k].addinginfo = sensorrow_adding[slst[k].sensorrowid];
                                }
                            }
                        }
                    }
                }
            }while(false);
            _db->close();
        }

        return res;
    }
    catch (std::exception& e)
    {
        ELOGE("GetLastestRecord exception:%s ", e.what());
    }
    catch (...)
    {
    }
    return false;

}

bool DBOperation::QueryRecordByTime(EOperDatabase type, QDateTime st, QDateTime et,QList<Record>& lst, QString* err)
{
    //��ԭʼ��¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
    {
        QSharedPointer<QSqlDatabase> _db;
        QSharedPointer<QMutex> _db_mutex;
        if(!GetOperDBParams(type, _db, _db_mutex))
        {
            return false;
        }
        QMutexLocker lk(_db_mutex.data());
        if(!_db->open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(_db->lastError().text()))) ;
            ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
            return false;
        }
        bool res = true;
        do 
        {//Ϊ�˹ر�����ʱ û��queryʹ�� //��ѯmain information
            QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
                                  FROM %1 WHERE TimeStart >= '%2' and TimeEnd <= '%3';")
                                  .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
            if(type == EOperDB_ShiftDB)
            {
                sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd, Date, Shift \
                              FROM %1 WHERE TimeStart >= '%2' and TimeEnd <= '%3';")
                              .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
            }
            //��ѯ����

            QSqlQuery query(*_db);
            if(!query.exec(sql) )
            {
                SAFE_SET(err, QString(QObject::tr("Query RTRecord database failure,the erro is %1").arg(query.lastError().text()))) ;
                res = false;
                break;
            }
            QMap<int, Record> mainrow_record;
            QMap<int, QList<MoldInfo> > mainrow_moldlst;
            QMap<int, QList<SensorInfo> > moldrow_sensorlst;
            QMap<int, QMap<QString, int> > sensorrow_adding;

            QSet<int> mainrowidlst, moldrowidlst, sensorrowidlst;

            while(query.next())
            {
                Record re;
                re.mainrowid    = query.value(0).toInt();
                re.id            = query.value(1).toString();
                re.inspected     = query.value(2).toInt();
                re.rejects       = query.value(3).toInt();
                re.defects       = query.value(4).toInt();
                re.autorejects   = query.value(5).toInt();
                re.dt_start      = QDateTime::fromString(query.value(6).toString(), "yyyy-MM-dd hh:mm:ss"); 
                re.dt_end        = QDateTime::fromString(query.value(7).toString(), "yyyy-MM-dd hh:mm:ss"); 
                if(type == EOperDB_ShiftDB)
                {
                    re.date = QDate::fromString(query.value(8).toString(), "yyyy-MM-dd");
                    re.shift = query.value(9).toInt();
                }
                if(re.inspected <= 0)
                {//������������0����������Ϣ
                    continue;
                }
                mainrowidlst.insert(re.mainrowid);
                lst.push_back(re);
            }
            if(!mainrowidlst.isEmpty())
            {
                if(!QueryMoldInfoByMainID(_db, mainrowidlst, mainrow_moldlst, moldrowidlst, err))
                {
                    res = false;
                    break;
                }
            }
            if(!moldrowidlst.isEmpty())
            {
                if(!QuerySensorInfoByMoldID(_db, moldrowidlst, moldrow_sensorlst, sensorrowidlst, err))
                {
                    res = false;
                    break;
                }
            }
            if(!sensorrowidlst.isEmpty())
            {
                if(!QuerySensorAddingInfoBySensorID(_db, sensorrowidlst, sensorrow_adding, err))
                {
                    res = false;
                    break;
                }
            }
            
            for(int i = 0; i < lst.size(); i++)
            {
                if(mainrow_moldlst.contains(lst[i].mainrowid))
                {
                    lst[i].moldinfo = mainrow_moldlst[lst[i].mainrowid];
                    QList<MoldInfo>& m = lst[i].moldinfo;
                    for(int j = 0; j < m.size(); j++)
                    {
                        if(moldrow_sensorlst.contains(m[j].moldrowid))
                        {
                            m[j].sensorinfo = moldrow_sensorlst[m[j].moldrowid];
                            QList<SensorInfo>& slst = m[j].sensorinfo;
                            for(int k = 0; k < slst.size(); k++)
                            {
                                if(sensorrow_adding.contains(slst[k].sensorrowid))
                                {
                                    slst[k].addinginfo = sensorrow_adding[slst[k].sensorrowid];
                                }
                            }
                        }
                    }
                }
            }
        }while(false);
        _db->close();
        return res;
    }
}

bool DBOperation::QueryMoldInfoByMainID(QSharedPointer<QSqlDatabase> _db,  const QSet<int>& mainrowidlst, QMap<int, QList<MoldInfo> > & moldinfo, QSet<int>& moldrowidlst, QString* err)
{
    QString rowlst = "";
    foreach(int id, mainrowidlst)
    {
        rowlst += QString("%1,").arg(id);
    }
    if(rowlst == "")
        return false;
    else
    {
        rowlst = rowlst.left(rowlst.size() -1);
    }
    QString sql = QString("SELECT ID, MoldID, Inspected, Rejects, Defects, Autoreject, TMainRowID FROM %1 WHERE TMainRowID in(%2);")
        .arg(tb_Mold).arg(rowlst);
    QSqlQuery mquery(*_db);
    if(!mquery.exec(sql))
    {//��ѯʧ��
        SAFE_SET(err, QString(QObject::tr("Query database failure,the error is %1").arg(mquery.lastError().text()))) ;
        return false;
    }
    while(mquery.next())
    {
        int mainrowid = -1;
        MoldInfo mold;
        mold.moldrowid      = mquery.value(0).toInt();
        mold.id            = mquery.value(1).toString();
        mold.inspected     = mquery.value(2).toInt();
        mold.rejects       = mquery.value(3).toInt();
        mold.defects       = mquery.value(4).toInt();
        mold.autorejects   = mquery.value(5).toInt();
        mainrowid        = mquery.value(6).toInt();

        if(mold.inspected <= 0)
        {
            continue;
        }

        moldrowidlst.insert(mold.moldrowid);

        if(moldinfo.contains(mainrowid))
        {
            moldinfo[mainrowid].append(mold);
        }
        else
        {
            QList<MoldInfo> t;
            t.append(mold);
            moldinfo.insert(mainrowid, t);
        }
    }
    return true;
}

bool DBOperation::QuerySensorInfoByMoldID(QSharedPointer<QSqlDatabase> _db,  const QSet<int>& moldrowidlst,QMap<int, QList<SensorInfo> >&  sensorinfo, QSet<int>& sensorrowidlst, QString* err)
{
    {//��ѯsensor��Ϣ
        QString rowlst = "";
        foreach(int id, moldrowidlst)
        {
            rowlst += QString("%1,").arg(id);
        }
        if(rowlst == "")
            return false;
        else
        {
            rowlst = rowlst.left(rowlst.size() -1);
        }

        QString sql = QString("SELECT ID, SensorID, Rejects, Defects, TMoldRowID FROM %1 WHERE TMoldRowID in (%2);")   
            .arg(tb_Sensor).arg(rowlst);
        //ELOGD(qPrintable(sql));
        QSqlQuery squery(*_db);
        if(!squery.exec(sql))
        {
            SAFE_SET(err, QString(QObject::tr("Query database failure,the error is %1").arg(squery.lastError().text()))) ;
            return false;
        }
        while(squery.next())
        {
            SensorInfo sensor;
            sensor.sensorrowid      = squery.value(0).toInt();
            sensor.id            = squery.value(1).toString();
            sensor.rejects       = squery.value(2).toInt();
            sensor.defects       = squery.value(3).toInt();
            int moldrowid        = squery.value(4).toInt();
            if(sensor.rejects <= 0)
            {
                continue;
            }

            sensorrowidlst.insert(sensor.sensorrowid);

            if(sensorinfo.contains(moldrowid))
            {
                sensorinfo[moldrowid].append(sensor);
            }
            else
            {
                QList<SensorInfo> t;
                t.append(sensor);
                sensorinfo.insert(moldrowid, t);
            }
        }
    }
    return true;
}

bool DBOperation::QuerySensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, const QSet<int>& sensorrowidlst, QMap<int, QMap<QString, int> >& addinginfo, QString* err)
{
    {//��ѯsensoradd��Ϣ
        QString rowlst = "";
        foreach(int id, sensorrowidlst)
        {
            rowlst += QString("%1,").arg(id);
        }
        if(rowlst == "")
            return false;
        else
        {
            rowlst = rowlst.left(rowlst.size() -1);
        }

        QString sql = QString("SELECT ID, CounterID, Nb,TSensorRowID FROM %1 WHERE TSensorRowID in(%2);")   
            .arg(tb_SensorAdd).arg(rowlst);
        //ELOGD(qPrintable(sql));
        QSqlQuery aquery(*_db);
        if(!aquery.exec(sql))
        {
            SAFE_SET(err, QString(QObject::tr("Query database failure,the erro is %1").arg(aquery.lastError().text()))) ;
            return false;
        }
        while(aquery.next())
        {
            QString counter_id;
            int nb;
            counter_id    = aquery.value(1).toString();
            nb            = aquery.value(2).toInt();
            int sensorrowid = aquery.value(3).toInt();

            if(addinginfo.contains(sensorrowid))
            {
                addinginfo[sensorrowid].insert(counter_id, nb);
            }
            else
            {
                QMap<QString, int> t;
                t.insert(counter_id, nb);
                addinginfo.insert(sensorrowid, t);
            }
        }
    }
    return true;
}

bool DBOperation::SaveRecord(EOperDatabase type, const Record& data, QString* err, bool isoutdb)
{
    if(data.inspected == 0)
    {//�������Ϊ0 ������
        return true;
    }
    bool res = true;
    {
        QSharedPointer<QSqlDatabase> _db;
        QSharedPointer<QMutex> _db_mutex;
        if(!GetOperDBParams(type, _db, _db_mutex))
        {
            return false;
        }

        if(!isoutdb)
        {
            //�����ݿ�
            if(!_db->open())
            {
                SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+_db->lastError().text())) ;
                ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
                return false;
            }

            if(!_db->transaction())
            {//��������ʧ��
                SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+_db->lastError().text())) ;
                _db->close();
                return false;
            }
        }

        do
        {//�����������
            //�½����ļ� ִ��һ�����ݱ�Ĵ������
            QSqlQuery query(*_db);

            //���ݲ��Ծ������������
            QString sql = QString("INSERT INTO %1(MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd) \
                                  Values('%2', %3, %4, %5, %6, '%7', '%8');").arg(tb_Main).arg(data.id).arg(data.inspected).arg(data.rejects).arg(data.defects)
                                  .arg(data.autorejects).arg(data.dt_start.toString("yyyy-MM-dd hh:mm:ss")).arg(data.dt_end.toString("yyyy-MM-dd hh:mm:ss"));
            if(type == EOperDB_ShiftDB)
            {
                sql = QString("INSERT INTO %1(MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd, Date, Shift) \
                              Values('%2', %3, %4, %5, %6, '%7', '%8', '%9',%10);").arg(tb_Main).arg(data.id).arg(data.inspected).arg(data.rejects).arg(data.defects)
                              .arg(data.autorejects).arg(data.dt_start.toString("yyyy-MM-dd hh:mm:ss")).arg(data.dt_end.toString("yyyy-MM-dd hh:mm:ss"))
                              .arg(data.date.toString("yyyy-MM-dd")).arg(data.shift);
            }
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

            if(!SaveMoldInfoByMainID(_db, mainrowid, data.moldinfo, err))
            {
                break;
            }

        }while(false); 

        if(!isoutdb)
        {
            if(!res)
            {
                _db->rollback();
                _db->close();
                return false;
            }
            _db->commit();
            _db->close();
        }

        return true;
    }

}
bool DBOperation::SaveMoldInfoByMainID(QSharedPointer<QSqlDatabase> _db, int mainrowid, const QList<MoldInfo>& moldinfo, QString* err)
{
    bool res = true;
    QSqlQuery query(*_db);
    foreach(MoldInfo mold, moldinfo)
    {
        if(mold.inspected == 0)
        {
            continue;
        }
        //ѭ������ÿ��ģ����Ϣ���ݣ�����ѯÿ��ģ���Ӧ��IDֵ
        QString sql = QString("INSERT INTO %1(TMainRowID, MoldID, Inspected, Rejects, Defects, Autoreject) \
                              VALUES(%2, '%3', %4, %5, %6, %7);").arg(tb_Mold).arg(mainrowid).arg(mold.id)
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
        if(!SaveSensorInfoByMoldID(_db, moldrowid, mold.sensorinfo, err))
        {
            res = false;
            break;
        }
    }
    return res;
}
bool DBOperation::SaveSensorInfoByMoldID(QSharedPointer<QSqlDatabase> _db, int moldrowid, const QList<SensorInfo>& sensorinfo, QString* err)
{
    bool res = true;
    QSqlQuery query(*_db);
    foreach(SensorInfo sensor, sensorinfo)
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
        QString sql = QString("INSERT INTO %1(TMoldRowID, SensorID, Rejects, Defects) VALUES(%2, '%3', %4, %5);")
            .arg(tb_Sensor).arg(moldrowid).arg(sensor.id).arg(sensor.rejects).arg(sensor.defects);
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
        if(!SaveSensorAddingInfoBySensorID(_db, sensorrowid, sensor.addinginfo, err))
        {
            res = false;
            break;
        }
    }
    return res;
}
bool DBOperation::SaveSensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, int sensorrowid, const QMap<QString, int>& addinginfo, QString* err)
{
    bool res = true;
    QSqlQuery query(*_db);
    foreach(auto k, addinginfo.keys())
    {
        //ѭ�����浥��ȱ�ݵĸ�����Ϣ����
        QString sql = QString("INSERT INTO %1(TSensorRowID, CounterID, Nb) VALUES(%2,'%3',%4);")
            .arg(tb_SensorAdd).arg(sensorrowid).arg(k).arg(addinginfo.value(k));
        if(!query.exec(sql))
        {
            SAFE_SET(err, QString(QObject::tr("Save SensorAdd Data Error, the Error is ")+query.lastError().text())) ;
            res = false;
            break;
        }
    }
    return res;
}

bool DBOperation::SaveShiftRecord(QSharedPointer<QSqlDatabase> _db, const Record& re, QString* err, bool isoutdb /*= false*/)
{
    if(re.inspected == 0)
    {//�������Ϊ0 ������
        return true;
    }
    bool res = true;
    if(!isoutdb)
    {
        //�����ݿ�
        if(!_db->open())
        {
            SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+_db->lastError().text())) ;
            ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
            return false;
        }

        if(!_db->transaction())
        {//��������ʧ��
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+_db->lastError().text())) ;
            _db->close();
            return false;
        }
    }

    do
    {//�����������
        //�½����ļ� ִ��һ�����ݱ�Ĵ������
        QSqlQuery query(*_db);

        //���ݲ��Ծ������������
        QString sql = QString("INSERT INTO %1(MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd, Date, Shift) \
                              Values('%2', %3, %4, %5, %6, '%7', '%8');").arg(tb_Main).arg(re.id).arg(re.inspected).arg(re.rejects).arg(re.defects)
                              .arg(re.autorejects).arg(re.dt_start.toString("yyyy-MM-dd hh:mm:ss")).arg(re.dt_end.toString("yyyy-MM-dd hh:mm:ss"))
                              .arg(re.date.toString("yyyy-MM-dd")).arg(re.shift);
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

        if(!SaveMoldInfoByMainID(_db, mainrowid, re.moldinfo, err))
        {
            break;
        }

    }while(false); 

    if(!isoutdb)
    {
        if(!res)
        {
            _db->rollback();
            _db->close();
            return false;
        }
        _db->commit();
        _db->close();
    }

    return true;
}

bool DBOperation::SaveRecordList(EOperDatabase type, QList<Record>& newlst, QString* err)
{
    {
        QSharedPointer<QSqlDatabase> _db;
        QSharedPointer<QMutex> _db_mutex;
        if(!GetOperDBParams(type, _db, _db_mutex))
        {
            return false;
        }

        QMutexLocker lk(_db_mutex.data());
        bool res = true;

        //�����ݿ�
        if(!_db->open())
        {
            SAFE_SET(err, QString(QObject::tr("Open Record database failure,the erro is %1").arg(_db->lastError().text()))) ;
            return false;
        }
        if(!_db->transaction())
        {//��������ʧ��
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+_db->lastError().text())) ;
            db->close();
            return false;
        }

        foreach(Record data, newlst)
        {//�����������
            if(!SaveRecord(type, data, err, true))
            {
                res = false;
                break;
            }
        } 

        if(!res)
        {
            _db->rollback();
        }
        else
        {
            _db->commit();
        }
        _db->close();
        return res;
    }

    //QSqlDatabase::removeDatabase(filename);
    return false;
}
