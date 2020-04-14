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
			//��ʼ�����ݿ�
			{//ʹ��SQL����� ���û�б��򴴽�
				_db->setDatabaseName(db_path+filelname);
				if(!_db->open())
				{
					SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(_db->lastError().text()))) ;
					return false;
				}
				//�½����ļ� ִ��һ�����ݱ�Ĵ������
				QSqlQuery query(*_db);
				bool res = false;
				do{
					if(isclassdb)
					{//��ο�
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

bool DBOperationB::checkSensorIDIsShouldSave( int id )
{
    if(cfgStrategy.bStrategyMode)
    {//������
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
    {//������
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
			{//Ϊ�˹ر�����ʱ û��queryʹ��
				//��ѯ����
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
		//�Ӽ�¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
		{
			QMutexLocker lk(_db_mutex.data());
			if(!_db->open())
			{
				QString err = QObject::tr("Open database failure,the erro is %1").arg(_db->lastError().text()) ;
				ELOGE(err.toLocal8Bit().constData());
				return false;
			}
			{//Ϊ�˹ر�����ʱ û��queryʹ��
				QString sql;
				if((ERecordType)type == ERT_TimeInterval)
				{
					QString tt = date.toString();
					QDateTime st(date, QTime(0,0,0));
					QDateTime et = st.addDays(1);

					//��ѯ����
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
	{//Ϊ�˹ر�����ʱ û��queryʹ��
		QString sql = QString("SELECT max(TimeEnd) as result FROM %1 ;").arg(tb_Main);
		QSqlQuery query(*_db);
		if(!query.exec(sql) || !query.next())
		{
			SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(query.lastError().text()))) ;
			_db->close();
			return false;
		}
		else
		{//��ȡ���
			if(!query.value(0).isNull())
			{//������ �� ��ΪNull
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
	{//Ϊ�˹ر�����ʱ û��queryʹ��
		QString sql = QString("SELECT min(TimeStart) as result FROM %1 ;").arg(tb_Main);
		QSqlQuery query(*_db);
		if(!query.exec(sql) || !query.next())
		{
			SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(query.lastError().text()))) ;
			_db->close();
			return false;
		}
		else
		{//��ȡ���
			if(!query.value(0).isNull())
			{//������ �� ��ΪNull
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
{//�豣֤���ݿ��ļ��Ѿ�����,�ҽṹ��ȷ
	/*************************************************************************
	* ��ȡ��ʼ��ʱ���
	* 1 �Ӽ�¼��� ��ȡ���¼�¼�Ľ�ֹʱ�� 
	* 2 ���û�м�¼,���ȡԭʼ��¼�ĵ�һ�����ݵ���ʼʱ�� 
	* 3 ���򷵻ص�ǰʱ��
	*************************************************************************/
	try
	{
		static bool isfirstlogdir = false;
		{   //1 �Ӽ�¼��� ��ȡ���¼�¼�Ľ�ֹʱ��
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
			//1 �Ӽ�¼��� ��ȡ�����¼����ʼʱ��
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
		//���ᵽ��ķ�֧
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
	//��ǰʱ��
	QString t = QDateTime::currentDateTime().addDays(outdatedays*-1).toString("yyyy-MM-dd hh:mm:ss");
	//��������� ���ֱ��ɾ���������� ����ɾ���������
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
	* ��ȡ��ʼ��ʱ���
	* 1 �Ӽ�¼��� ��ȡ���¼�¼�Ľ�ֹʱ�� 
	* 2 ���û�м�¼,���ȡԭʼ��¼�ĵ�һ�����ݵ���ʼʱ�� 
	* 3 ���򷵻ص�ǰʱ��
	*************************************************************************/
	try
	{
		static bool isfirstlogdir = false;
		{   //1 �Ӽ�¼��� ��ȡ���¼�¼�Ľ�ֹʱ��
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
			//1 ��ʵʱ��¼��� ��ȡ�����¼����ʼʱ��
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
		//���ᵽ��ķ�֧
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
	//��ǰʱ��
	QString t = time.toString("yyyy-MM-dd hh:mm:ss");
	//QString et = time.addSecs(ETimeInterval2Min(eti)*60).toString("yyyy-MM-dd hh:mm:ss");
	//��������� ���ֱ��ɾ���������� ����ɾ���������
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
		{//ʵʱ���ݿ�
			_db = rtdb;
			_db_mutex = rtdb_mutex;
		}
		break;
	case EOperDB_TimeIntervalDB:
		{//ʱ�������ݿ�
			_db = db;
			_db_mutex = db_mutex;
		}
		break;
	case EOperDB_ShiftDB:
		{//���ͳ�Ƽ�¼���ݿ�
			_db = shiftdb;
			_db_mutex = shiftdb_mutex;
		}
		break;
	default://��֧�ֵ����ݿ�ѡ��
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
	default://��֧�ֵļ�¼ѡ��
		ELOGD(QObject::tr("Not support Selection:%1").arg((int)type).toLocal8Bit().constData());
		return false;
		break;
	}
	return true;
}
