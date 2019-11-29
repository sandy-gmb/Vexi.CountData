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
{//需保证数据库文件已经存在,且结构正确
    /*************************************************************************
    * 获取起始的时间点
    * 1 从记录表格 获取最新记录的截止时间 
    * 2 如果没有记录,则获取原始记录的第一条数据的起始时间 
    *************************************************************************/
    QString filename = db_filename;
    {   //1 从记录表格 获取最新记录的截止时间
        QMutexLocker lk(db_mutex.data());
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
            return false;
        }
        {//为了关闭连接时 没有query使用
            QString sql = QString("SELECT max(TimeEnd) as result FROM %1 ;").arg(tb_Main);
            QSqlQuery query(sql, db);
            if(!query.exec() || !query.next())
            {
                SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                db.close();
                return false;
            }
            else
            {//获取结果
                if(!query.value(0).isNull())
                {//没有数据,结果为NULL
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
        //1 从记录表格 获取最新记录的截止时间
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
            return false;
        }
        {//为了关闭连接时 没有query使用
            QString sql = QString("SELECT min(TimeStart) as result FROM %1;").arg(tb_Main);
            QSqlQuery query(sql, db);
            if(!query.exec() || !query.next())
            {
                SAFE_SET(err, QString(QObject::tr("Query database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
                db.close();
                return false;
            }
            else
            {//获取结果
                if(!query.value(0).isNull())
                {//没有数据,结果为NULL
                    t = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss");
                    //QString st = t.toString();
                    //ELOGD("GetLastT RT_T:%s", qPrintable(t.toString()));
                    db.close();
                    return true;
                }
            }
        }
    }
    //不会到达的分支
    ELOGE("%s can't access branch", __FUNCTION__);
    return false;
}

bool GenerateRecord::QueryRecordByTime(QDateTime st, QDateTime et,QList<Record>& lst, QString* err)
{
    //从原始记录数据库获取大于等于指定时间的对应记录
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
        {//为了关闭连接时 没有query使用
            //查询主表
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
                {//过检总数大于0才有其他信息
                    sql = QString("SELECT ID, MoldID, Inspected, Rejects, Defects, Autoreject FROM %1 WHERE TMainRowID=%2;")
                        .arg(tb_Mold).arg(mainrowid);
                    //ELOGD(qPrintable(sql));
                    QSqlQuery mquery(db);
                    if(!mquery.exec(sql))
                    {//查询失败
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
                        {//查询sensor信息
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
                                {//查询sensoradd信息
                                    
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
* @brief  :  CalcGenerateRecordTime 返回的是起始时间
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
    //计算第一个时间间隔的起始时间
    QDateTime t(start.date(), QTime(0,0));
    //从凌晨12点开始计算起始时间属于的区间,
    int passd = t.secsTo(start)/60/ti;
    QDateTime st = t;
    st = st.addSecs(passd*ti*60);
    QStringList tslst;

    while(st.secsTo(end) > ti*60 )
    {//存在需要生成的记录区间
        tslst.push_back(st.toString());

        lst.push_back(st);
        st = st.addSecs(ti*60);
    }

    if(!lst.isEmpty())
    {
        //最后一个是截止时间
        lst.push_back(st);
    }
}

bool GenerateRecord::StatisticsRecord(QList<QDateTime>& tlst, QList<Record>& lst, QList<Record>& newlst, QString* err)
{//传进来的数据不能为空
    if(tlst.size() < 2 || lst.isEmpty())
    {//时间间隔为空 或者元数据为空
        //由于时间分界点至少是两个(起始+结束),因此判定小于2
        SAFE_SET(err, QObject::tr("the data isn't enough"));
        return false;
    }
    /**************************************************
    * 1 同时遍历时间分界点和数据节点
    * 注:默认数据是有序的(时间升序 统计记录时间升序),
    ***************************************************/

    int j = 0;
    for(int i = 0; i < tlst.size()-1; i++)
    {//遍历时间分界点
        Record r;
        r.dt_start = tlst[i];
        r.dt_end = tlst[i+1];
        //统计该时间段的数据
        for(; j < lst.size();j++)
        {
            QString t1s = r.dt_start.toString();
            QString t1e = r.dt_end.toString();
            QString t2s = lst[j].dt_start.toString();
            QString t2e = lst[j].dt_end.toString();
            if(lst[j].dt_end <= r.dt_end && lst[j].dt_end >= r.dt_start)
            {//原本只用判定截止时间在被选时间段之前
                r.id = lst[j].id;
                r.MergeData(lst[j]);
            }
            else if(lst[j].dt_end > r.dt_end)
            {//出现下一个时间段的数据,则退出,组织下一个统计记录
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

        //打开数据库
        db.setDatabaseName(db_path+filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("Open database file:%1 failure,the erro is %2").arg(filename).arg(db.lastError().text()))) ;
            return false;
        }
        if(!db.transaction())
        {//开启事务失败
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+db.lastError().text())) ;
            db.close();
            return false;
        }

        foreach(Record data, newlst)
        {//开启事务操作
            //新建的文件 执行一遍数据表的创建语句
            QSqlQuery query(db);

            //根据策略决定保存的数据
            QString sql = QString("INSERT INTO %1(MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd) \
                                  Values('%2', %3, %4, %5, %6, '%7', '%8');").arg(tb_Main).arg(data.id).arg(data.inspected).arg(data.rejects).arg(data.defects)
                                  .arg(data.autorejects).arg(data.dt_start.toString("yyyy-MM-dd hh:mm:ss")).arg(data.dt_end.toString("yyyy-MM-dd hh:mm:ss"));

            //保存主表数据，并查询主表对应的ID值
            if(!query.exec(sql))
            {
                SAFE_SET(err, QString(QObject::tr("Save Machine Data Error, the Error is ")+query.lastError().text())) ;
                res = false;
                break;
            }
            //查询主表对应的ID值，新增的ID值最大
            if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Main)))
            {
                SAFE_SET(err, QString(QObject::tr("Query Main Data ID Error: ")+query.lastError().text())) ;
                res = false;
                break;
            }
            //不可能出现的情形 
            if(!query.next() || query.isNull(0))
            {
                SAFE_SET(err, QString(QObject::tr("query in a exception, no main data")+query.lastError().text())) ;
                res = false;
                break;
            }
            int mainrowid = query.value(0).toInt();

            foreach(MoldInfo mold, data.moldinfo)
            {
                //循环保存每个模板信息数据，并查询每个模板对应的ID值
                sql = QString("INSERT INTO %1(TMainRowID, MoldID, Inspected, Rejects, Defects, Autoreject) \
                              VALUES(%2, %3, %4, %5, %6, %7);").arg(tb_Mold).arg(mainrowid).arg(mold.id)
                              .arg(mold.inspected).arg(mold.rejects).arg(mold.defects).arg(mold.autorejects);
                if(!query.exec(sql))
                {
                    SAFE_SET(err, QString(QObject::tr("Save Mold Data Error, the Error is ")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                //查询模板表对应的ID值，新增的ID值最大
                if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Mold)))
                {
                    SAFE_SET(err, QString(QObject::tr("Query Mold Data ID Error: ")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                //不可能出现的情形 
                if(!query.next() || query.isNull(0))
                {
                    SAFE_SET(err, QString(QObject::tr("query in a exception, no mold data")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                int moldrowid = query.value(0).toInt();

                foreach(SensorInfo sensor, mold.sensorinfo)
                {
                    //循环保存单个模板的所有缺陷信息数据，并记录每种缺陷数据的ID
                    sql = QString("INSERT INTO %1(TMoldRowID, SensorID, Rejects, Defects) \
                                  VALUES(%2, %3, %4, %5);").arg(tb_Sensor).arg(moldrowid).arg(sensor.id).arg(sensor.rejects)
                                  .arg(sensor.defects);
                    if (!query.exec(sql))
                    {
                        SAFE_SET(err, QString(QObject::tr("Save Sensor Data Error, the Error is ")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    //查询模板表对应的ID值，新增的ID值最大
                    if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Sensor)))
                    {
                        SAFE_SET(err, QString(QObject::tr("Query Sensor Data ID Error: ")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    //不可能出现的情形 
                    if(!query.next() || query.isNull(0))
                    {
                        SAFE_SET(err, QString(QObject::tr("query in a exception, no sensor data")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    int sensorrowid = query.value(0).toInt();

                    foreach(SensorAddingInfo adding, sensor.addinginfo)
                    {
                        //循环保存单个缺陷的附加信息数据
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
    //当前时间
    QString t = QDateTime::currentDateTime().addDays(outdatedays*-1).toString("yyyy-MM-dd hh:mm:ss");
    //由于有外键 因此直接删除主表数据 即可删除相关数据
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
    //默认30天过期
    outdatedays = 30;

    db_mutex = mx;
    rtdb_mutex = rt_mx;
    db = QSqlDatabase::addDatabase("QSQLITE", "GenerateRecord");
}

void GenerateRecord::work()
{
    /***************************************************************************************************
    * 函数实现:
    *  定时删除过期原始记录
    *
    * 1 检查记录数据库的主表,得到 最新的日志截止时间 QDateTime t 有可能没有
    * 2 根据t (没有t,则以实时记录的第一条的起始时间为准),计算需要添加的记录间隔,
    * 3 检查实时数据数据库,得到所有在t之后的数据,如果没有t,则获取所有
    * 4 根据时间间隔 和 原始数据,统计得到记录数据
    * 5 根据统计记录数据,生成每一条记录,放到记录数据库中
    *
    ****************************************************************************************************/
    bool res = false;

    QDateTime dellast = QDateTime::currentDateTime();
    dellast = dellast.addDays(-2);
    QString err;
    while (runflg) 
    {//生成记录

        do 
        {
            //定时删除记录 每天执行一次,
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
            //1 检查记录数据库的主表,得到 最新的日志截止时间 QDateTime t,如果获取失败,则从原始记录表获取第一条记录的起始时间
            res = GetLastestRecord(t, &err);
            if(!res)
            {
                ELOGD(qPrintable(err));
            }
            //ELOGD("GetLastT: %s", qPrintable(t.toString()));

            //2 根据截止时间 和时间间隔 计算需要生成记录的时间间隔
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

            //3 检查时间和时间间隔 查看是否有最新记录需要生成
            QList<Record> record_lst;
            res = QueryRecordByTime(t, et, record_lst, &err);
            if(record_lst.isEmpty())
            {
                break;
            }

            if(!runflg){
                break;
            }

            //4 根据生成区间列表统计每个区间的数据
            QList<Record> statistic_record_lst;
            res = StatisticsRecord(timelst, record_lst, statistic_record_lst, &err);
            if(!runflg){
                break;
            }
            //5 生成记录
            res = GenerateRecordToDB(statistic_record_lst, &err);

        } while (false);

       
        int cnt = 5000;
        while(cnt > 0)
        {
            int single = 200;
            //生成记录很耗时,并且实时性要求不高,因此可以设置为5 sec
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
