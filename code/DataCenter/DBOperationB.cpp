#include "DBOperationB.h"
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

bool DBOperationB::Init(QString* err)
{
    bool res = Init(rtdb, rtdb_filename, err) && Init(db, db_filename, err) && Init(shiftdb, shiftdb_filename, err, true);
    if(res)
    {
        Start();
    }
    return res;
}

bool DBOperationB::Init(QSharedPointer<QSqlDatabase> _db, QString filelname, QString* err /*= nullptr*/, bool isclassdb /*= false*/)
{
    try
    {
        {
            QDir d ;
            if(!d.exists(db_path))
            {
                bool t = d.mkpath(db_path);
            }
            QString t;
            //初始化数据库
            {//使用SQL语句检查 如果没有表则创建
                _db->setDatabaseName(db_path+filelname);
                if(!_db->open())
                {
                    SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(_db->lastError().text()))) ;
                    return false;
                }
                //新建的文件 执行一遍数据表的创建语句
                QSqlQuery query(*_db);
                bool res = false;
                do{
                    if(isclassdb)
                    {//班次库
                        res = query.exec(create_tb_ShiftMain_SQL);
                    }
                    else
                    {
                        res = query.exec(create_tb_Main_SQL);
                    }
                    if(!res)
                    {
                        break;
                    }
                    res = false;
                    res = query.exec(create_tb_Mold_SQL);
                    if(!res)
                    {
                        break;
                    }
                    res = false;
                    res = query.exec(create_tb_Sensor_SQL);
                    if(!res)
                    {
                        break;
                    }
                    res = false;
                    res = query.exec(create_tb_SensorAdd_SQL);
                    if(!res)
                    {
                        break;
                    }
                } while (false);
                if(!res)
                {
                    t = QString(QObject::tr("Initial database failure,the erro is %1").arg(query.lastError().text())) ;
                    ELOGE(qPrintable(t));
                    SAFE_SET(err, t);
                    _db->close();
                    return false;
                }
            }
            _db->close();
        }

        //QSqlDatabase::removeDatabase(filelname);
        return true;
    }
    catch (std::exception& e)
    {
        ELOGE("init database exception:%s", e.what());
    }
    catch (...)
    {
    }
    return false;
}

bool DBOperationB::checkSensorIDIsShouldSave(QString id )
{
    if(cfgStrategy.bStrategyMode)
    {//白名单
        foreach(auto i, cfgStrategy.lSensorIDs)
        {
            if(i == id)
            {
                return true;
            }
        }
        return false;
    }
    else
    {//黑名单
        foreach(auto i, cfgStrategy.lSensorIDs)
        {
            if(i == id)
            {
                return false;
            }
        }
        return true;
    }
}

void DBOperationB::Start()
{
    thd->start();
    emit sig_start();
}

void DBOperationB::Stop()
{
    ELOGD("DBOperationB::Stop()");
    work->runflg = false;
    thd->wait(1000);
}

DBOperationB::DBOperationB( QObject* parent /*= nullptr*/ )
    : QObject(parent)
{
    db = QSharedPointer< QSqlDatabase> (new QSqlDatabase( QSqlDatabase::addDatabase("QSQLITE", "Record"))) ;
    rtdb = QSharedPointer< QSqlDatabase> (new QSqlDatabase( QSqlDatabase::addDatabase("QSQLITE", "RTRecord"))) ;
    shiftdb = QSharedPointer< QSqlDatabase> (new QSqlDatabase( QSqlDatabase::addDatabase("QSQLITE", "ShiftRecord"))) ;

    db_mutex = QSharedPointer<QMutex>(new QMutex(QMutex::Recursive));
    rtdb_mutex = QSharedPointer<QMutex>(new QMutex(QMutex::Recursive));
    shiftdb_mutex = QSharedPointer<QMutex>(new QMutex(QMutex::Recursive));

    work = new GenerateRecord(this);
    thd = new QThread(this);

    work->moveToThread(thd);
    connect(this, SIGNAL(sig_start()), work, SLOT(work()));
    connect(thd, SIGNAL(finished()), work, SLOT(deleteLater()));
}

DBOperationB::~DBOperationB()
{
    Stop();
    thd->terminate();
}

bool DBOperationB::GetAllDate(int type, QList<QDate>& lst )
{
    try
    {
        lst.clear();
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
                QString err = QObject::tr("Open databasefailure,the erro is %1").arg(_db->lastError().text()) ;
                ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
                return false;
            }
            {//为了关闭连接时 没有query使用
                //查询主表
                QString sql = QString("SELECT distinct (date(TimeStart)) as date From %1").arg(tb_Main);
                QSqlQuery query(*_db);
                if(!query.exec(sql) )
                {
                    QString err = QObject::tr("Query database failure,the erro is %1").arg(query.lastError().text()) ;
                    ELOGE(err.toLocal8Bit().constData());

                    _db->close();
                    return false;
                }
                while(query.next())
                {
                    QDate t = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
                    lst.push_back(t);
                }
            }
            _db->close();
        }
        return true;
    }
    catch (std::exception& e)
    {
        ELOGE("GetAllDate exception:%s ", e.what());
    }
    catch (...)
    {
    }
    return false;
    
}

bool DBOperationB::GetRecordTimeListByDay(int type, QDate date, QList<QDateTime>& stlst, QList<QDateTime>& etlst )
{
    try
    {
        QSharedPointer<QSqlDatabase> _db;
        QSharedPointer<QMutex> _db_mutex;
        if(!GetRecordDBParams((ERecordType)type, _db, _db_mutex))
        {
            return false;
        }

        stlst.clear();
        etlst.clear();
        //从记录数据库获取大于等于指定时间的对应记录
        {
            QMutexLocker lk(_db_mutex.data());
            if(!_db->open())
            {
                QString err = QObject::tr("Open database failure,the erro is %1").arg(_db->lastError().text()) ;
                ELOGE(err.toLocal8Bit().constData());
                return false;
            }
            {//为了关闭连接时 没有query使用
                QString sql;
                if((ERecordType)type == ERT_TimeInterval)
                {
                    QString tt = date.toString();
                    QDateTime st(date, QTime(0,0,0));
                    QDateTime et = st.addDays(1);

                    //查询主表
                    sql = QString("SELECT TimeStart,TimeEnd From %1 WHERE TimeStart >= '%2' and TimeEnd <= '%3';")
                        .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
                }
                else
                {
                    sql = QString("SELECT TimeStart,TimeEnd From %1 WHERE Date='%2';").arg(tb_Main).arg(date.toString("yyyy-MM-dd"));
                }

                QSqlQuery query(*_db);
                if(!query.exec(sql) )
                {
                    QString err = QObject::tr("Query database failure,the erro is %1").arg(query.lastError().text()) ;
                    ELOGE(err.toLocal8Bit().constData());

                    _db->close();
                    return false;
                }
                while(query.next())
                {
                    QDateTime t = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss");
                    stlst.push_back(t);
                    t = QDateTime::fromString(query.value(1).toString(), "yyyy-MM-dd hh:mm:ss");
                    etlst.push_back(t);
                }
            }
            _db->close();
        }

        return true;
    }
    catch (std::exception& e)
    {
        ELOGE("GetRecordListByDay Date:%s TimeList.size:%d exception:%s ", date.toString().toStdString().c_str(), stlst.size(), e.what());
    }
    catch (...)
    {
    }
    return false;
}

bool DBOperationB::GetLastestRecordEndTime(QSharedPointer<QSqlDatabase> _db, QDateTime& t, QString* err)
{
    if(!_db->open())
    {
        SAFE_SET(err, QString(QObject::tr("Open Record database failure,the erro is %1").arg(_db->lastError().text()))) ;
        ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
        return false;
    }
    {//为了关闭连接时 没有query使用
        QString sql = QString("SELECT max(TimeEnd) as result FROM %1 ;").arg(tb_Main);
        QSqlQuery query(*_db);
        if(!query.exec(sql) || !query.next())
        {
            SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(query.lastError().text()))) ;
            _db->close();
            return false;
        }
        else
        {//获取结果
            if(!query.value(0).isNull())
            {//有数据 且 不为Null
                t = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss");
                _db->close();
                return true;
            }
        }
    }
    _db->close();
    return false;
}

bool DBOperationB::GetOldestRecordStartTime(QSharedPointer<QSqlDatabase> _db, QDateTime& t, QString* err)
{
    if(!_db->open())
    {
        SAFE_SET(err, QString(QObject::tr("Open Record database failure,the erro is %1").arg(_db->lastError().text()))) ;
        ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
        return false;
    }
    {//为了关闭连接时 没有query使用
        QString sql = QString("SELECT min(TimeStart) as result FROM %1 ;").arg(tb_Main);
        QSqlQuery query(*_db);
        if(!query.exec(sql) || !query.next())
        {
            SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(query.lastError().text()))) ;
            _db->close();
            return false;
        }
        else
        {//获取结果
            if(!query.value(0).isNull())
            {//有数据 且 不为Null
                t = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss");
                _db->close();
                return true;
            }
        }
    }
    _db->close();
    return false;
}

bool DBOperationB::GetLatestRecordEndTime(QDateTime& t, QString* err)
{//需保证数据库文件已经存在,且结构正确
    /*************************************************************************
    * 获取起始的时间点
    * 1 从记录表格 获取最新记录的截止时间 
    * 2 如果没有记录,则获取原始记录的第一条数据的起始时间 
    * 3 否则返回当前时间
    *************************************************************************/
    try
    {
        static bool isfirstlogdir = false;
        {   //1 从记录表格 获取最新记录的截止时间
            QMutexLocker lk(db_mutex.data());
            if(!isfirstlogdir)
            {
                ELOGD("current Dir:%s", QDir::current().path().toStdString().c_str());
                isfirstlogdir = true;
            }
            if(GetLastestRecordEndTime(db, t, err))
            {
                return true;
            }
            //
        }
        {
            QMutexLocker lk(rtdb_mutex.data());
            //1 从记录表格 获取最早记录的起始时间
            if(GetOldestRecordStartTime(rtdb, t, err))
            {
                return true;
            }
            else
            {
                t = QDateTime::currentDateTime();
                return true;
            }
        }
        //不会到达的分支
        ELOGE("%s can't access branch", __FUNCTION__);
        return false;
    }
    catch (std::exception& e)
    {
        ELOGE("%s exception:%s", __FUNCTION__, e.what());
    }
    catch (...)
    {
    }
    return false;

}

void DBOperationB::DeleteOutdatedData(int outdatedays)
{
    //当前时间
    QString t = QDateTime::currentDateTime().addDays(outdatedays*-1).toString("yyyy-MM-dd hh:mm:ss");
    //由于有外键 因此直接删除主表数据 即可删除相关数据
    {
        QMutexLocker lk(rtdb_mutex.data());
        if(!rtdb->open())
        {
            ELOGD(QObject::tr("Open RTRecord database failure,the erro is %1").arg(db->lastError().text()).toLocal8Bit().constData());
            ELOGD("Open database failure,the erro is %s", qPrintable(rtdb->lastError().text()));
            return ;
        }
        rtdb->exec("PRAGMA foreign_keys = ON;");
        {
            QString sql = QString("DELETE FROM %1 WHERE TimeEnd < '%2';").arg(tb_Main).arg(t);
            QSqlQuery query(*rtdb);
            if(!query.exec(sql))
            {
                ELOGD(QObject::tr("Delete RTRecord database data failure,the erro is %1").arg(query.lastError().text()).toLocal8Bit().constData());
            }

        }
        rtdb->close();
    }
}

bool DBOperationB::GetLastestShiftRecordEndTime(QDateTime& t, QString* err)
{
    /*************************************************************************
    * 获取起始的时间点
    * 1 从记录表格 获取最新记录的截止时间 
    * 2 如果没有记录,则获取原始记录的第一条数据的起始时间 
    * 3 否则返回当前时间
    *************************************************************************/
    try
    {
        static bool isfirstlogdir = false;
        {   //1 从记录表格 获取最新记录的截止时间
            QMutexLocker lk(shiftdb_mutex.data());
            if(!isfirstlogdir)
            {
                ELOGD("current Dir:%s", QDir::current().path().toStdString().c_str());
                isfirstlogdir = true;
            }
            if(GetLastestRecordEndTime(shiftdb, t, err))
            {
                return true;
            }
            //
        }
        {
            QMutexLocker lk(rtdb_mutex.data());
            //1 从实时记录表格 获取最早记录的起始时间
            if(GetOldestRecordStartTime(rtdb, t, err))
            {
                return true;
            }
            else
            {
                t = QDateTime::currentDateTime();
                return true;
            }
        }
        //不会到达的分支
        ELOGE("%s can't access branch", __FUNCTION__);
        return false;
    }
    catch (std::exception& e)
    {
        ELOGE("%s exception:%s", __FUNCTION__, e.what());
    }
    catch (...)
    {
    }
    return false;
}

bool DBOperationB::DeleteRecordAfterTime(EOperDatabase type, QDateTime time)
{
    static bool isfirst = false;
    //当前时间
    QString t = time.toString("yyyy-MM-dd hh:mm:ss");
    //QString et = time.addSecs(ETimeInterval2Min(eti)*60).toString("yyyy-MM-dd hh:mm:ss");
    //由于有外键 因此直接删除主表数据 即可删除相关数据
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
            if(!isfirst)
            {
                ELOGD(QObject::tr("Open record database failure,the erro is %1").arg(_db->lastError().text()).toLocal8Bit().constData());
                ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
                isfirst = true;
            }
            return false;
        }
        _db->exec("PRAGMA foreign_keys = ON;");
        {
            QString sql = QString("DELETE FROM %1 WHERE  TimeEnd >= '%2';").arg(tb_Main).arg(t);
            QSqlQuery query(*_db);
            if(!query.exec(sql))
            {
                if(!isfirst)
                {
                    ELOGD(QObject::tr("Delete record database data failure,the erro is %1").arg(query.lastError().text()).toLocal8Bit().constData());
                    isfirst = true;
                }
            }

        }
        _db->close();
    }
    return true;
}


bool DBOperationB::GetOperDBParams( EOperDatabase type, QSharedPointer<QSqlDatabase>& _db, QSharedPointer<QMutex>& _db_mutex)
{
    switch(type)
    {
    case EOperDB_RuntimeDB:
        {//实时数据库
            _db = rtdb;
            _db_mutex = rtdb_mutex;
        }
        break;
    case EOperDB_TimeIntervalDB:
        {//时间间隔数据库
            _db = db;
            _db_mutex = db_mutex;
        }
        break;
    case EOperDB_ShiftDB:
        {//班次统计记录数据库
            _db = shiftdb;
            _db_mutex = shiftdb_mutex;
        }
        break;
    default://不支持的数据库选项
        ELOGD(QObject::tr("Delete database data, input a invalid selection:%1").arg(type).toLocal8Bit().constData());
        return false;
        break;
    }
    return true;
}

bool DBOperationB::GetRecordDBParams(ERecordType type, QSharedPointer<QSqlDatabase>& _db, QSharedPointer<QMutex>& _db_mutex)
{
    EOperDatabase t;
    if(!GetDBOperType( type, t))
    {
        return false;
    }
    return GetOperDBParams(t, _db, _db_mutex);
}

bool DBOperationB::GetDBOperType(ERecordType type, EOperDatabase& optype)
{
    switch(type)
    {
    case  ERT_TimeInterval:
        {
            optype = EOperDB_TimeIntervalDB;
        }
        break;
    case ERT_Shift:
        {
            optype = EOperDB_ShiftDB;
        }
        break;
    default://不支持的记录选项
        ELOGD(QObject::tr("Not support Selection:%1").arg((int)type).toLocal8Bit().constData());
        return false;
        break;
    }
    return true;
}
