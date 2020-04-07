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

bool DBOperation::Init(QString* err)
{
    bool res = Init(rtdb, rtdb_filename, err) && Init(db, db_filename, err) && Init(shiftdb, shiftdb_filename, err, true);
    if(res)
    {
        Start();
    }
    return res;
}

bool DBOperation::Init(QSharedPointer<QSqlDatabase> _db, QString filelname, QString* err /*= nullptr*/, bool isclassdb /*= false*/)
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

bool DBOperation::checkSensorIDIsShouldSave( int id )
{
    if(cfgStrategy.bStrategyMode)
    {//白名单
        foreach(int i, cfgStrategy.lSensorIDs)
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
        foreach(int i, cfgStrategy.lSensorIDs)
        {
            if(i == id)
            {
                return false;
            }
        }
        return true;
    }
}

void DBOperation::Start()
{
    thd->start();
    emit sig_start();
}

void DBOperation::Stop()
{
	ELOGD("SaveDataToDB::Stop()");
    work->runflg = false;
    thd->wait(1000);
}

DBOperation::DBOperation( QObject* parent /*= nullptr*/ )
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

DBOperation::~DBOperation()
{
    Stop();
    thd->terminate();
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

		//从记录数据库获取大于等于指定时间的对应记录
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

void DBOperation::RecordConfigChanged()
{
	QDateTime t = QDateTime::currentDateTime();
	DeleteRecordAfterTime(EOperDB_TimeIntervalDB, t);
	DeleteRecordAfterTime(EOperDB_ShiftDB, t);
}

void DBOperation::OnDataConfChange(const DataCenterConf& cfg)
{
	cfgStrategy = cfg.strategy;
	work->cfgSystem = cfg.syscfg;
	work->eti = cfg.eTimeInterval;
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
			{//为了关闭连接时 没有query使用
				//查询主表
				QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
									  FROM %1 ORDER BY TimeEnd desc limit 1;").arg(tb_Main);
				if(type == (int)ERT_Shift)
				{
					sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd, Date, Shift \
								  FROM %1 ORDER BY TimeEnd desc limit 1;").arg(tb_Main);
				}
				QSqlQuery query(*_db);
				if(!query.exec(sql) )
				{
					SAFE_SET(err, QString(QObject::tr("Query databasefailure,the erro is %1").arg(query.lastError().text()))) ;
					ELOGD("Query databasefailure,the erro is %s", qPrintable( query.lastError().text()));
					_db->close();
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
					if(type == ERT_Shift)
					{
						re.date        = QDate::fromString(query.value(8).toString(), "yyyy-MM-dd"); 
						QString t = re.date.toString();
						 t = query.value(8).toString();
						re.shift = query.value(9).toInt();
					}
					if(re.inspected > 0)
					{//过检总数大于0才有其他信息
						if(!QueryMoldInfoByMainID(_db, mainrowid, re.moldinfo, err))
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

ETimeInterval DBOperation::GetTimeInterval()
{
    return work->eti;
}

void DBOperation::SetTimeInterval( ETimeInterval timeinterval )
{
    work->eti = timeinterval;
	//删除今天的所有数据
	DeleteRecordAfterTime(EOperDB_TimeIntervalDB, QDateTime::currentDateTime());
}

bool DBOperation::GetAllDate(int type, QList<QDate>& lst )
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

bool DBOperation::GetRecordTimeListByDay(int type, QDate date, QList<QTime>& stlst, QList<QTime>& etlst )
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
					QTime t = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss").time();
					stlst.push_back(t);
					t = QDateTime::fromString(query.value(1).toString(), "yyyy-MM-dd hh:mm:ss").time();
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

bool DBOperation::QueryRecordByTime(EOperDatabase type, QDateTime st, QDateTime et,QList<Record>& lst, QString* err)
{
	//从原始记录数据库获取大于等于指定时间的对应记录
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
		{//为了关闭连接时 没有query使用 //查询main information
			QString sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd \
								  FROM %1 WHERE TimeStart >= '%2' and TimeEnd <= '%3';")
								  .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
			if(type == EOperDB_ShiftDB)
			{
				sql = QString("SELECT ID, MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd, Date, Shift \
									  FROM %1 WHERE TimeStart >= '%2' and TimeEnd <= '%3';")
									  .arg(tb_Main).arg(st.toString("yyyy-MM-dd hh:mm:ss")).arg(et.toString("yyyy-MM-dd hh:mm:ss"));
			}
			//查询主表
			
			QSqlQuery query(*_db);
			if(!query.exec(sql) )
			{
				SAFE_SET(err, QString(QObject::tr("Query RTRecord database failure,the erro is %1").arg(query.lastError().text()))) ;
				_db->close();
				return false;
			}
			QMap<int, Record> mainrow_record;
			QMap<int, QList<MoldInfo> > mainrow_moldlst;
			QMap<int, QList<SensorInfo> > moldrow_sensorlst;
			QMap<int, QMap<int, int> > sensorrow_adding;

			QList<int> mainrowidlst, moldrowidlst, sensorrowidlst;

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
				{//过检总数大于0才有其他信息
					continue;
				}
				lst.push_back(re);
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
							m[j].sensorinfo = moldrow_sensorlst[m[j].moldrowid]);
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
	}

	return res;
}

bool DBOperation::QueryMoldInfoByMainID(QSharedPointer<QSqlDatabase> _db,  const QList<int>& mainrowidlst, QMap<int, QList<MoldInfo> > & moldinfo, QList<int>& moldrowidlst, QString* err)
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
	{//查询失败
		SAFE_SET(err, QString(QObject::tr("Query database failure,the error is %1").arg(mquery.lastError().text()))) ;
		return false;
	}
	while(mquery.next())
	{
		int mainrowid = -1;
		MoldInfo mold;
		mold.moldrowid      = mquery.value(0).toInt();
		mold.id            = mquery.value(1).toInt();
		mold.inspected     = mquery.value(2).toInt();
		mold.rejects       = mquery.value(3).toInt();
		mold.defects       = mquery.value(4).toInt();
		mold.autorejects   = mquery.value(5).toInt();
		mainrowid		= mquery.value(6).toInt();
		
		if(mold.inspected <= 0)
		{
			continue;
		}

		moldrowidlst.append(mold.id);

		if(moldinfo.contains(mainrowid))
		{
			moldinfo[mainrowid].append(mold)]
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

bool DBOperation::QuerySensorInfoByMoldID(QSharedPointer<QSqlDatabase> _db,  const QList<int>& moldrowidlst,QMap<int, QList<SensorInfo> >&  sensorinfo, QList<int>& sensorrowidlst, QString* err)
{
	{//查询sensor信息
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
			sensor.id            = squery.value(1).toInt();
			sensor.rejects       = squery.value(2).toInt();
			sensor.defects       = squery.value(3).toInt();
			int moldrowid        = squery.value(4).toInt();
			if(sensor.inspected <= 0)
			{
				continue;
			}

			sensorrowidlst.append(sensor.id);

			if(sensorinfo.contains(moldrowid))
			{
				sensorinfo[moldrowid].append(mold)]
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

bool DBOperation::QuerySensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, const QList<int>& sensorrowidlst, QMap<int, QList<int, int> >& addinginfo, QString* err)
{
	{//查询sensoradd信息
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
			int counter_id, nb;
			counter_id    = aquery.value(1).toInt();
			nb            = aquery.value(2).toInt();
			int sensorrowid = aquery.value(3).toInt();

			if(sensorinfo.contains(sensorrowid))
			{
				addinginfo[sensorrowid].insert(counter_id, nb);
			}
			else
			{
				QMap<int, int> t;
				t.insert(counter_id, nb);
				addinginfo.insert(moldrowid, t);
			}
		}
	}
	return true;
}

bool DBOperation::SaveRecord(EOperDatabase type, const Record& data, QString* err, bool isoutdb)
{
	if(data.inspected == 0)
	{//如果数据为0 不保存
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
			//打开数据库
			if(!_db->open())
			{
				SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+_db->lastError().text())) ;
				ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
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
			if(type == EOperDB_ShiftDB)
			{
				sql = QString("INSERT INTO %1(MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd, Date, Shift) \
								  Values('%2', %3, %4, %5, %6, '%7', '%8', '%9',%10);").arg(tb_Main).arg(data.id).arg(data.inspected).arg(data.rejects).arg(data.defects)
								  .arg(data.autorejects).arg(data.dt_start.toString("yyyy-MM-dd hh:mm:ss")).arg(data.dt_end.toString("yyyy-MM-dd hh:mm:ss"))
								  .arg(data.date.toString("yyyy-MM-dd")).arg(data.shift);
			}
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
bool DBOperation::SaveSensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, int sensorrowid, const QMap<int, int>& addinginfo, QString* err)
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

bool DBOperation::QueryShiftRecordByClass(QSharedPointer<QSqlDatabase> _db, const QDate& d,int shift, Record record, QString* err)
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
								  FROM %1 WHERE Date = '%2' and shift=%3;")
								  .arg(tb_Main).arg(d.toString("yyyy-MM-dd")).arg(shift);
			QSqlQuery query(*_db);
			if(!query.exec(sql) )
			{
				SAFE_SET(err, QString(QObject::tr("Query Shift Record database failure,the erro is %1").arg(query.lastError().text()))) ;
				shiftdb->close();
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
				re.date = d;
				re.shift = shift;
				if(re.inspected > 0)
				{//过检总数大于0才有其他信息
					if(!QueryMoldInfoByMainID(shiftdb, mainrowid, re.moldinfo, err))
					{
						_db->close();
						return false;
					}
				}
			}
			else
			{
				SAFE_SET(err, QString(QObject::tr("Query Shift Record database failure,the goal record don't exist.").arg(query.lastError().text()))) ;
				_db->close();
				return false;
			}
		}
		_db->close();
	}

	return true;
}

bool DBOperation::SaveShiftRecord(QSharedPointer<QSqlDatabase> _db, const Record& re, QString* err, bool isoutdb /*= false*/)
{
	if(re.inspected == 0)
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
			ELOGD("Open database failure,the erro is %s", qPrintable(_db->lastError().text()));
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
		QString sql = QString("INSERT INTO %1(MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd, Date, Shift) \
							  Values('%2', %3, %4, %5, %6, '%7', '%8');").arg(tb_Main).arg(re.id).arg(re.inspected).arg(re.rejects).arg(re.defects)
							  .arg(re.autorejects).arg(re.dt_start.toString("yyyy-MM-dd hh:mm:ss")).arg(re.dt_end.toString("yyyy-MM-dd hh:mm:ss"))
							  .arg(re.date.toString("yyyy-MM-dd")).arg(re.shift);
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

bool DBOperation::GetLastestRecordEndTime(QSharedPointer<QSqlDatabase> _db, QDateTime& t, QString* err)
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
bool DBOperation::GetOldestRecordStartTime(QSharedPointer<QSqlDatabase> _db, QDateTime& t, QString* err)
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


bool DBOperation::GetLatestRecordEndTime(QDateTime& t, QString* err)
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

void DBOperation::DeleteOutdatedData(int outdatedays)
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

bool DBOperation::GetLastestShiftRecordEndTime(QDateTime& t, QString* err)
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


bool DBOperation::DeleteRecordAfterTime(EOperDatabase type, QDateTime time)
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

bool DBOperation::GetOperDBParams( EOperDatabase type, QSharedPointer<QSqlDatabase>& _db, QSharedPointer<QMutex>& _db_mutex)
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

bool DBOperation::GetRecordDBParams(ERecordType type, QSharedPointer<QSqlDatabase>& _db, QSharedPointer<QMutex>& _db_mutex)
{
	EOperDatabase t;
	if(!GetDBOperType( type, t))
	{
		return false;
	}
	return GetOperDBParams(t, _db, _db_mutex);
}

bool DBOperation::GetDBOperType(ERecordType type, EOperDatabase& optype)
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

		//打开数据库
		if(!_db->open())
		{
			SAFE_SET(err, QString(QObject::tr("Open Record database failure,the erro is %1").arg(_db->lastError().text()))) ;
			return false;
		}
		if(!_db->transaction())
		{//开启事务失败
			SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+_db->lastError().text())) ;
			db->close();
			return false;
		}

		foreach(Record data, newlst)
		{//开启事务操作
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
