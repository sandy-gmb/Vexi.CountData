#include "SaveDataToDB.h"
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

bool SaveDataToDB::Init(QString* err)
{
    bool res = Init(rtdb, rtdb_filename, err) && Init(db, db_filename, err);
    if(res)
    {
        Start();
    }
    return res;
}

bool SaveDataToDB::Init(QSharedPointer<QSqlDatabase> _db, QString filelname, QString* err /*= nullptr*/ )
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
					res = query.exec(create_tb_Main_SQL);
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

bool SaveDataToDB::SaveData(const XmlData& data, QString* err)
{
    try
    {
		QMutexLocker lk(rtdb_mutex.data());
		return SaveRecord(rtdb, data, err);
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

void SaveDataToDB::Start()
{
    thd->start();
    emit sig_start();
}

void SaveDataToDB::Stop()
{
	ELOGD("SaveDataToDB::Stop()");
    work->runflg = false;
    thd->wait(1000);
}

SaveDataToDB::SaveDataToDB( QObject* parent /*= nullptr*/ )
    : QObject(parent)
{
	db = QSharedPointer< QSqlDatabase> (new QSqlDatabase( QSqlDatabase::addDatabase("QSQLITE", "Record"))) ;
	rtdb = QSharedPointer< QSqlDatabase> (new QSqlDatabase( QSqlDatabase::addDatabase("QSQLITE", "RTRecord"))) ;

    db_mutex = QSharedPointer<QMutex>(new QMutex(QMutex::Recursive));
    rtdb_mutex = QSharedPointer<QMutex>(new QMutex(QMutex::Recursive));

    work = new GenerateRecord(this);
    thd = new QThread(this);

    work->moveToThread(thd);
    connect(this, SIGNAL(sig_start()), work, SLOT(work()));
    connect(thd, SIGNAL(finished()), work, SLOT(deleteLater()));
}

SaveDataToDB::~SaveDataToDB()
{
    Stop();
    thd->terminate();
}

bool SaveDataToDB::GetRecordByTime( QDateTime st, QDateTime end, Record& data )
{
	try
	{
		//从记录数据库获取大于等于指定时间的对应记录
		QMutexLocker lk(db_mutex.data());
		QList<Record> relst;
		QString err;
		if(!QueryRecordByTime(db, st, end, relst, &err))
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

bool SaveDataToDB::GetLastestRecord( Record& data, QString* err )
{
	try
	{
		bool res = true;
		//从原始记录数据库获取大于等于指定时间的对应记录
		{
			QMutexLocker lk(db_mutex.data());
			if(!db->open())
			{
				SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(db->lastError().text()))) ;
				return false;
			}
			{//为了关闭连接时 没有query使用
				//查询主表
				QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
									  FROM %1 ORDER BY TimeEnd desc limit 1;").arg(tb_Main);
				QSqlQuery query(*db);
				if(!query.exec(sql) )
				{
					SAFE_SET(err, QString(QObject::tr("Query databasefailure,the erro is %1").arg(query.lastError().text()))) ;
					db->close();
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
					{//过检总数大于0才有其他信息
						if(!QueryMoldInfoByMainID(db, mainrowid, re.moldinfo, err))
						{
							res = false;
						}
					}
					data = re;
				}
				else
				{
					res = false;
				}
			}
			db->close();
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

ETimeInterval SaveDataToDB::GetTimeInterval()
{
    return work->eti;
}

void SaveDataToDB::SetTimeInterval( ETimeInterval timeinterval )
{
    work->eti = timeinterval;
	//删除今天的所有数据
	QDateTime t(QDate::currentDate(), QTime(0,0));
	work->DeleteRecordAfterTime(t);
}

bool SaveDataToDB::GetAllDate( QList<QDate>& lst )
{
	try
	{
		lst.clear();
		{
			QMutexLocker lk(db_mutex.data());
			if(!db->open())
			{
				QString err = QObject::tr("Open databasefailure,the erro is %1").arg(db->lastError().text()) ;
				ELOGE(err.toLocal8Bit().constData());
				return false;
			}
			{//为了关闭连接时 没有query使用
				//查询主表
				QString sql = QString("SELECT distinct (date(TimeStart)) as date From %1").arg(tb_Main);
				QSqlQuery query(*db);
				if(!query.exec(sql) )
				{
					QString err = QObject::tr("Query database failure,the erro is %1").arg(query.lastError().text()) ;
					ELOGE(err.toLocal8Bit().constData());

					db->close();
					return false;
				}
				while(query.next())
				{
					QDate t = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
					lst.push_back(t);
				}
			}
			db->close();
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

bool SaveDataToDB::GetRecordListByDay( QDate date, QList<QTime>& stlst, QList<QTime>& etlst )
{
	try
	{
		QString tt = date.toString();

		QDateTime st(date, QTime(0,0,0));
		QDateTime et = st.addDays(1);

		stlst.clear();
		etlst.clear();
		//从记录数据库获取大于等于指定时间的对应记录
		{
			QMutexLocker lk(db_mutex.data());
			if(!db->open())
			{
				QString err = QObject::tr("Open database failure,the erro is %1").arg(db->lastError().text()) ;
				ELOGE(err.toLocal8Bit().constData());
				return false;
			}
			{//为了关闭连接时 没有query使用
				//查询主表
				QString sql = QString("SELECT TimeStart,TimeEnd From %1 WHERE TimeStart >= '%2' and TimeEnd <= '%3';")
					.arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
				QSqlQuery query(*db);
				if(!query.exec(sql) )
				{
					QString err = QObject::tr("Query database failure,the erro is %1").arg(query.lastError().text()) ;
					ELOGE(err.toLocal8Bit().constData());

					db->close();
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
			db->close();
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

bool SaveDataToDB::QueryRecordByTime(QSharedPointer<QSqlDatabase> _db, QDateTime st, QDateTime et,QList<Record>& lst, QString* err)
{
	//从原始记录数据库获取大于等于指定时间的对应记录
	{
		if(!_db->open())
		{
			SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(_db->lastError().text()))) ;
			return false;
		}
		{//为了关闭连接时 没有query使用 //查询main information
			//查询主表
			QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
								  FROM %1 WHERE TimeStart >= '%2' and TimeStart < '%3';")
								  .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
			QSqlQuery query(*_db);
			if(!query.exec(sql) )
			{
				SAFE_SET(err, QString(QObject::tr("Query RTRecord database failure,the erro is %1").arg(query.lastError().text()))) ;
				_db->close();
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
					if(!QueryMoldInfoByMainID(_db, mainrowid, re.moldinfo, err))
					{
						_db->close();
						return false;
					}
				}
				lst.push_back(re);
			}
		}
		_db->close();
	}

	return true;
}

bool SaveDataToDB::QueryMoldInfoByMainID(QSharedPointer<QSqlDatabase> _db, int mainrowid, QList<MoldInfo>& moldinfo, QString* err)
{
	QString sql = QString("SELECT ID, MoldID, Inspected, Rejects, Defects, Autoreject FROM %1 WHERE TMainRowID=%2;")
		.arg(tb_Mold).arg(mainrowid);
	QSqlQuery mquery(*_db);
	if(!mquery.exec(sql))
	{//查询失败
		SAFE_SET(err, QString(QObject::tr("Query database failure,the error is %1").arg(mquery.lastError().text()))) ;
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

		if(!QuerySensorInfoByMoldID(_db, moldrowid, mold.sensorinfo, err))
		{
			return false;
		}

		moldinfo.push_back(mold);
	}
	return true;
}

bool SaveDataToDB::QuerySensorInfoByMoldID(QSharedPointer<QSqlDatabase> _db, int moldrowid, QList<SensorInfo>& sensorinfo, QString* err)
{
	{//查询sensor信息
		QString sql = QString("SELECT ID, SensorID, Rejects, Defects FROM %1 WHERE TMoldRowID=%2")   
			.arg(tb_Sensor).arg(moldrowid);
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
			int sensorrowid      = squery.value(0).toInt();
			sensor.id            = squery.value(1).toInt();
			sensor.rejects       = squery.value(2).toInt();
			sensor.defects       = squery.value(3).toInt();
			if(sensor.rejects <= 0)
			{
				continue;
			}
			if(!QuerySensorAddingInfoBySensorID(_db, sensorrowid, sensor.addinginfo, err))
			{
				return false;
			}

			sensorinfo.push_back(sensor);
		}
	}
	return true;
}

bool SaveDataToDB::QuerySensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, int sensorrowid, QMap<int, int>& addinginfo, QString* err)
{
	{//查询sensoradd信息
		QString sql = QString("SELECT ID, CounterID, Nb FROM %1 WHERE TSensorRowID=%2")   
			.arg(tb_SensorAdd).arg(sensorrowid);
		//ELOGD(qPrintable(sql));
		QSqlQuery aquery(*_db);
		if(!aquery.exec(sql))
		{
			SAFE_SET(err, QString(QObject::tr("Query database failure,the erro is %1").arg(aquery.lastError().text()))) ;
			return false;
		}
		while(aquery.next())
		{
			int counter_id, nb;
			counter_id    = aquery.value(1).toInt();
			nb            = aquery.value(2).toInt();

			addinginfo.insert(counter_id, nb);
		}
	}
	return true;
}

bool SaveDataToDB::SaveRecord(QSharedPointer<QSqlDatabase> _db,const Record& data, QString* err, bool isoutdb)
{
	if(data.inspected == 0)
	{//如果数据为0 不保存
		return true;
	}
	bool res = true;
	if(!isoutdb)
	{
		//打开数据库
		if(!_db->open())
		{
			SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+_db->lastError().text())) ;
			return false;
		}
	
		if(!_db->transaction())
		{//开启事务失败
			SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+_db->lastError().text())) ;
			_db->close();
			return false;
		}
	}

	do
	{//开启事务操作
		//新建的文件 执行一遍数据表的创建语句
		QSqlQuery query(*_db);

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
bool SaveDataToDB::SaveMoldInfoByMainID(QSharedPointer<QSqlDatabase> _db, int mainrowid, const QList<MoldInfo>& moldinfo, QString* err)
{
	bool res = true;
	QSqlQuery query(*_db);
	foreach(MoldInfo mold, moldinfo)
	{
		if(mold.inspected == 0)
		{
			continue;
		}
		//循环保存每个模板信息数据，并查询每个模板对应的ID值
		QString sql = QString("INSERT INTO %1(TMainRowID, MoldID, Inspected, Rejects, Defects, Autoreject) \
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
		if(!SaveSensorInfoByMoldID(_db, moldrowid, mold.sensorinfo, err))
		{
			res = false;
			break;
		}
	}
	return res;
}
bool SaveDataToDB::SaveSensorInfoByMoldID(QSharedPointer<QSqlDatabase> _db, int moldrowid, const QList<SensorInfo>& sensorinfo, QString* err)
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
		{//根据策略，缺陷数据不用保存
			continue;
		}
		//循环保存单个模板的所有缺陷信息数据，并记录每种缺陷数据的ID
		QString sql = QString("INSERT INTO %1(TMoldRowID, SensorID, Rejects, Defects) VALUES(%2, %3, %4, %5);")
			.arg(tb_Sensor).arg(moldrowid).arg(sensor.id).arg(sensor.rejects).arg(sensor.defects);
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
		if(!SaveSensorAddingInfoBySensorID(_db, sensorrowid, sensor.addinginfo, err))
		{
			res = false;
			break;
		}
	}
	return res;
}
bool SaveDataToDB::SaveSensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, int sensorrowid, const QMap<int, int>& addinginfo, QString* err)
{
	bool res = true;
	QSqlQuery query(*_db);
	foreach(int k, addinginfo.keys())
	{
		//循环保存单个缺陷的附加信息数据
		QString sql = QString("INSERT INTO %1(TSensorRowID, CounterID, Nb) VALUES(%2,%3,%4);")
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

bool SaveDataToDB::GetLastestRecordEndTime(QSharedPointer<QSqlDatabase> _db, QDateTime& t, QString* err)
{
	if(!_db->open())
	{
		SAFE_SET(err, QString(QObject::tr("Open Record database failure,the erro is %1").arg(_db->lastError().text()))) ;
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
bool SaveDataToDB::GetOldestRecordStartTime(QSharedPointer<QSqlDatabase> _db, QDateTime& t, QString* err)
{
	if(!_db->open())
	{
		SAFE_SET(err, QString(QObject::tr("Open Record database failure,the erro is %1").arg(_db->lastError().text()))) ;
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
