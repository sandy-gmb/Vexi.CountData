#include "SaveDataToDB.h"

#include <QString>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>

#include <QDomDocument>

const QString& strategyfile = QObject::tr("./Record/strategy.xml");

const QString& db_path = QObject::tr("./Record/");
const QString& db_filename = QObject::tr("record.db3");
//表名 主表|模表|缺陷表|附加信息表
const QString& tb_Main = QObject::tr("T_Main");
const QString& tb_Mold = QObject::tr("T_Mold");
const QString& tb_Sensor = QObject::tr("T_Sensor");
const QString& tb_SensorAdd = QObject::tr("T_SensorAdd");

const QString& create_tb_Main_SQL = QString("CREATE TABLE IF NOT EXISTS T_Main (\
                                            ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                            MachineID    VARCHAR,\
                                            Inspected    INTEGER,\
                                            Rejects      INTEGER,\
                                            Defects      INTEGER,\
                                            Autoreject   INTEGER,\
                                            [TimeStart] DATETIME,\
                                            [TimeEnd]   DATETIME );");
const QString& create_tb_Mold_SQL = QString("CREATE TABLE IF NOT EXISTS T_Mold (\
                                            ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                            TMainRowID   INTEGER,\
                                            MoldID       INTEGER,\
                                            Inspected    INTEGER,\
                                            Rejects      INTEGER,\
                                            Defects      INTEGER,\
                                            Autoreject   INTEGER );");
const QString& create_tb_Sensor_SQL = QString("CREATE TABLE IF NOT EXISTS T_Sensor (\
                                              ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                              TMoldRowID   INTEGER,\
                                              SensorID     INTEGER,\
                                              Rejects      INTEGER,\
                                              Defects      INTEGER );");
const QString& create_tb_SensorAdd_SQL = QString("CREATE TABLE IF NOT EXISTS T_SensorAdd (\
                                               ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                               TSensorRowID INTEGER,\
                                               CounterID    INTEGER,\
                                               Nb           VARCHAR );");

bool SaveDataToDB::Init(QString* err)
{
    m_islogevery_data = false;
    {
        //初始化数据库
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "record_db");
        QFile f(db_path+db_filename);
        if(!f.exists())
        {//没有打开时会自动创建文件，因此打开时，需要执行创建语句保证所有数据表已创建
            QDir d ;
            if(!d.exists(db_path))
            {
                bool t = d.mkpath(db_path);
            }
            db.setDatabaseName(db_path+db_filename);
            if(!db.open())
            {
                SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+db.lastError().text())) ;
                return false;
            }
            //新建的文件 执行一遍数据表的创建语句
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
                SAFE_SET(err, QString(QObject::tr("Initial the database failure, the erro is ")+ query.lastError().text())) ;
                db.close();
                return false;
            }
        }
        else{//有则打开
            db.setDatabaseName(db_path+db_filename);
            if(!db.open())
            {
                SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+db.lastError().text())) ;
                return false;
            }
        }
        db.close();
    }

    QSqlDatabase::removeDatabase("record_db");
    
    return true;
}

bool SaveDataToDB::SaveData(const XmlData& data, QString* err)
{
    {
        bool res = true;

        //打开数据库
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "record_db");
        db.setDatabaseName(db_path+db_filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+db.lastError().text())) ;
            return false;
        }
        if(!db.transaction())
        {//开启事务失败
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+db.lastError().text())) ;
            db.close();
            return false;
        }

        do
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
                    if(!checkSensorIDIsShouldSave(sensor.id))
                    {//根据策略，缺陷数据不用保存
                        continue;
                    }
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
    
    QSqlDatabase::removeDatabase("record_db");
    ParserStrategyFile(true);
    return true;
}

void SaveDataToDB::ParserStrategyFile(bool first)
{
    if(first && !QFile::exists(strategyfile))
    {
        strategy_mode = true;
        sensorids.push_back(10);    //结石
        sensorids.push_back(34);    //小结石

        SaveStrategyData();
        lastmodify = QDateTime::currentDateTime();
        return;
    }

    QDomDocument doc("strategyfile");
    QFile file(strategyfile);
    if (!file.open(QIODevice::ReadOnly))
    {//文件打开失败，如果是第一次 则设置为默认数据 否则不管
        if(first)
        {
            strategy_mode = true;
            sensorids.push_back(10);    //结石
            sensorids.push_back(34);    //小结石

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
        {//文件无改动
            return;
        }
    }
    lastmodify = t;
    //以下为XML文件标准格式
    ///*<strategy>
    //    <!--策略模式 true表示白名单模式 其他表示黑名单-->
    //    <mode>true</mode>
    //    <sensor>
    //    <id>10</id>
    //    <id>34</id>
    //    </sensor>
    //    <islogall>true</islogall>
    //    </strategy>*/

    //解析策略文件
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    // 解析XML文件
    QDomElement root = doc.documentElement();
    if(root.tagName() != "strategy")
    {
        //"XML data the top tag isn't strategy,the xml data has incorrect format"
        return ;
    }
    if(!root.hasChildNodes())
    {//没有子节点数据
        //"Paser XML data,Root Node, No Child";
        return ;
    }
    QDomElement mode = root.firstChildElement("mode");
    if(mode.isNull())
    {//没有此节点
        //"Paser XML data,No Called mode child Node";
        return ;
    }
    strategy_mode = (mode.text() == "true")?true:false;
    QDomElement logall = root.firstChildElement("islogall");
    if(logall.isNull())
    {//没有此节点
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
        return false; //新建文件打开失败
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
    stream.setCodec("UTF-8"); //使用utf-8格式

    doc.save(stream, 4, QDomNode::EncodingFromTextStream);
    file.close();
    return true;
}

bool SaveDataToDB::checkSensorIDIsShouldSave( int id )
{
    if(strategy_mode)
    {//白名单
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
    {//黑名单
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


