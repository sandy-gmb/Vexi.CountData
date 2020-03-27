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
			//if(!f.exists())
			{//ʹ��SQL����� ���û�б��򴴽�
				if(!db->open())
				{
					SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(db->lastError().text()))) ;
					return false;
				}
				//�½����ļ� ִ��һ�����ݱ�Ĵ������
				QSqlQuery query(*db);
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
					db->close();
					return false;
				}
			}
			db->close();
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
    ELOGD("SaveDataToDB::Stop()");
}

SaveDataToDB::SaveDataToDB( QObject* parent /*= nullptr*/ )
    : QObject(parent)
{
	db = QSharedPointer< QSqlDatabase> (new QSqlDatabase( QSqlDatabase::addDatabase("QSQLITE", "Record"))) ;
	db->setDatabaseName(db_path+db_filename);
	rtdb = QSharedPointer< QSqlDatabase> (new QSqlDatabase( QSqlDatabase::addDatabase("QSQLITE", "RTRecord"))) ;
	rtdb->setDatabaseName(db_path+rtdb_filename);

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
		//�Ӽ�¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
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
		//��ԭʼ��¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
		{
			QMutexLocker lk(db_mutex.data());
			if(!db->open())
			{
				SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(db->lastError().text()))) ;
				return false;
			}
			{//Ϊ�˹ر�����ʱ û��queryʹ��
				//��ѯ����
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
					{//������������0����������Ϣ
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
	//ɾ���������������
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
			{//Ϊ�˹ر�����ʱ û��queryʹ��
				//��ѯ����
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
		//�Ӽ�¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
		{
			QMutexLocker lk(db_mutex.data());
			if(!db->open())
			{
				QString err = QObject::tr("Open database failure,the erro is %1").arg(db->lastError().text()) ;
				ELOGE(err.toLocal8Bit().constData());
				return false;
			}
			{//Ϊ�˹ر�����ʱ û��queryʹ��
				//��ѯ����
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
	//��ԭʼ��¼���ݿ��ȡ���ڵ���ָ��ʱ��Ķ�Ӧ��¼
	{
		if(!_db->open())
		{
			SAFE_SET(err, QString(QObject::tr("Open database failure,the erro is %1").arg(_db->lastError().text()))) ;
			return false;
		}
		{//Ϊ�˹ر�����ʱ û��queryʹ�� //��ѯmain information
			//��ѯ����
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
				{//������������0����������Ϣ
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
	{//��ѯʧ��
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
	{//��ѯsensor��Ϣ
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
	{//��ѯsensoradd��Ϣ
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
		//ѭ������ÿ��ģ����Ϣ���ݣ�����ѯÿ��ģ���Ӧ��IDֵ
		QString sql = QString("INSERT INTO %1(TMainRowID, MoldID, Inspected, Rejects, Defects, Autoreject) \
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
		{//���ݲ��ԣ�ȱ�����ݲ��ñ���
			continue;
		}
		//ѭ�����浥��ģ�������ȱ����Ϣ���ݣ�����¼ÿ��ȱ�����ݵ�ID
		QString sql = QString("INSERT INTO %1(TMoldRowID, SensorID, Rejects, Defects) VALUES(%2, %3, %4, %5);")
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
bool SaveDataToDB::SaveSensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> _db, int sensorrowid, const QMap<int, int>& addinginfo, QString* err)
{
	bool res = true;
	QSqlQuery query(*_db);
	foreach(int k, addinginfo.keys())
	{
		//ѭ�����浥��ȱ�ݵĸ�����Ϣ����
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
