#include "DBOperation_Recur.h"
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

bool DBOperation_Recur::SaveData(const XmlData& data, QString* err)
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


DBOperation_Recur::DBOperation_Recur( QObject* parent /*= nullptr*/ )
    : DBOperationB(parent)
{
	
}

DBOperation_Recur::~DBOperation_Recur()
{
}

bool DBOperation_Recur::GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data )
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
//
//void DBOperation_Recur::RecordConfigChanged()
//{
//	QDateTime t = QDateTime::currentDateTime();
//	DeleteRecordAfterTime(EOperDB_TimeIntervalDB, t);
//	DeleteRecordAfterTime(EOperDB_ShiftDB, t);
//}
//
//void DBOperation_Recur::OnDataConfChange(const DataCenterConf& cfg)
//{
//	cfgStrategy = cfg.strategy;
//	work->cfgSystem = cfg.syscfg;
//	work->eti = cfg.eTimeInterval;
//}

bool DBOperation_Recur::GetLastestRecord(int type, Record& data, QString* err )
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

bool DBOperation_Recur::QueryRecordByTime(EOperDatabase type, QDateTime st, QDateTime et,QList<Record>& lst, QString* err)
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
				if(re.inspected > 0)
				{//过检总数大于0才有其他信息
					if(!QueryMoldInfoByMainID(_db, re.mainrowid, re.moldinfo, err))
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

bool DBOperation_Recur::QueryMoldInfoByMainID(QSharedPointer<QSqlDatabase> _db, int mainrowid, QList<MoldInfo>& moldinfo, QString* err)
{
	QString sql = QString("SELECT ID, MoldID, Inspected, Rejects, Defects, Autoreject FROM %1 WHERE TMainRowID=%2 order by MoldID asc;")
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
		mold.moldrowid      = mquery.value(0).toInt();
		mold.id            = mquery.value(1).toInt();
		mold.inspected     = mquery.value(2).toInt();
		mold.rejects       = mquery.value(3).toInt();
		mold.defects       = mquery.value(4).toInt();
		mold.autorejects   = mquery.value(5).toInt();
		if(mold.inspected <= 0)
		{
			continue;
		}

		if(!QuerySensorInfoByMoldID(_db, mold.moldrowid, mold.sensorinfo, err))
		{
			return false;
		}

		moldinfo.push_back(mold);
	}
	return true;
}

bool DBOperation_Recur::QuerySensorInfoByMoldID(QSharedPointer<QSqlDatabase> _db, int moldrowid, QList<SensorInfo>& sensorinfo, QString* err)
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
			sensor.sensorrowid      = squery.value(0).toInt();
			sensor.id            = squery.value(1).toInt();
			sensor.rejects       = squery.value(2).toInt();
			sensor.defects       = squery.value(3).toInt();
			if(sensor.rejects <= 0)
			{
				continue;
			}
			if(!QuerySensorAddingInfoBySensorID(_db, sensor.sensorrowid, sensor.addinginfo, err))
			{
				return false;
			}

			sensorinfo.push_back(sensor);
		}
	}
	return true;
}

bool DBOperation_Recur::QuerySensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, int sensorrowid, QMap<int, int>& addinginfo, QString* err)
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

bool DBOperation_Recur::SaveRecord(EOperDatabase type, const Record& data, QString* err, bool isoutdb)
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

bool DBOperation_Recur::SaveMoldInfoByMainID(QSharedPointer<QSqlDatabase> _db, int mainrowid, const QList<MoldInfo>& moldinfo, QString* err)
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

bool DBOperation_Recur::SaveSensorInfoByMoldID(QSharedPointer<QSqlDatabase> _db, int moldrowid, const QList<SensorInfo>& sensorinfo, QString* err)
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

bool DBOperation_Recur::SaveSensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, int sensorrowid, const QMap<int, int>& addinginfo, QString* err)
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

bool DBOperation_Recur::SaveShiftRecord(QSharedPointer<QSqlDatabase> _db, const Record& re, QString* err, bool isoutdb /*= false*/)
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

bool DBOperation_Recur::SaveRecordList(EOperDatabase type, QList<Record>& newlst, QString* err)
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
