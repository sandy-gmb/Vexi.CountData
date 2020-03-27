#include "GenerateRecord.h"
#include "SaveDataTodb.h"

#include <QString>
#include <QObject>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
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
#include <QDebug>

bool GenerateRecord::GetLastestRecordEndTime(QDateTime& t, QString* err)
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
			if(m_pthis->GetLastestRecordEndTime(db, t, err))
			{
				return true;
			}
			//
		}
		{
			QMutexLocker lk(rtdb_mutex.data());
			//1 从记录表格 获取最新记录的截止时间
			if(m_pthis->GetLastestRecordEndTime(db, t, err))
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
    QDateTime end = QDateTime::currentDateTime().addSecs(ti*60);
    //计算第一个时间间隔的起始时间
    QDateTime t(start.date(), QTime(0,0));
    //从凌晨12点开始计算起始时间属于的区间,
    int passd = t.secsTo(start)/60/ti;
    QDateTime st = t;
    st = st.addSecs(passd*ti*60);
    QStringList tslst;

    while(st.secsTo(end) > ti*60 )
    {//存在需要生成的记录区间 需要生成的记录区间需要包含当前的区间
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
    {
        bool res = true;
        QMutexLocker lk(db_mutex.data());

        //打开数据库
        if(!db->open())
        {
			SAFE_SET(err, QString(QObject::tr("Open Record database failure,the erro is %1").arg(db->lastError().text()))) ;
            return false;
        }
        if(!db->transaction())
        {//开启事务失败
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+db->lastError().text())) ;
            db->close();
            return false;
        }

        foreach(Record data, newlst)
        {//开启事务操作
            if(!m_pthis->SaveRecord(db, data, err, true))
			{
				res = false;
				break;
			}
        } 

        if(!res)
        {
            db->rollback();
        }
        else
        {
            db->commit();
        }
        db->close();
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
        if(!rtdb->open())
        {
            ELOGD(QObject::tr("Open RTRecord database failure,the erro is %1").arg(db->lastError().text()).toLocal8Bit().constData());
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

bool GenerateRecord::DeleteRecordAfterTime(QDateTime time)
{
	static bool isfirst = false;
	//当前时间
	QString t = time.toString("yyyy-MM-dd hh:mm:ss");
	//QString et = time.addSecs(ETimeInterval2Min(eti)*60).toString("yyyy-MM-dd hh:mm:ss");
	//由于有外键 因此直接删除主表数据 即可删除相关数据
	{
		QMutexLocker lk(db_mutex.data());
		if(!db->open())
		{
			if(!isfirst)
			{
				ELOGD(QObject::tr("Open record database failure,the erro is %1").arg(db->lastError().text()).toLocal8Bit().constData());
				isfirst = true;
			}
			return false;
		}
		{
			QString sql = QString("DELETE FROM %1 WHERE  TimeEnd >= '%2';").arg(tb_Main).arg(t);
			QSqlQuery query(*db);
			if(!query.exec(sql))
			{
				if(!isfirst)
				{
					ELOGD(QObject::tr("Delete record database data failure,the erro is %1").arg(query.lastError().text()).toLocal8Bit().constData());
					isfirst = true;
				}
			}

		}
		db->close();
	}
	return true;
}
GenerateRecord::GenerateRecord( QObject *parent /*= nullptr*/ )
    : QObject(nullptr)
    , runflg(true)
{
	m_pthis = (SaveDataToDB*)parent;

	db = m_pthis->db;
	rtdb = m_pthis->rtdb;
	db_mutex = m_pthis->db_mutex;
	rtdb_mutex = m_pthis->rtdb_mutex;

    eti = ETI_30_Min;
    //默认30天过期
    outdatedays = 30;

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
            res = GetLastestRecordEndTime(t, &err);
            if(!res)
            {
                ELOGD(qPrintable(err));
            }

            //2 根据截止时间 和时间间隔 计算需要生成记录的时间间隔
            QList<QDateTime> timelst;
			//ELOGD("%s", t.toString().toStdString().c_str());
            CalcGenerateRecordTime(t, eti, timelst);
            if(timelst.isEmpty())
            {
				DeleteRecordAfterTime(QDateTime::currentDateTime());
                continue;
            }
            if(!runflg){
                break;
            }
            QDateTime et = timelst.last();

            //3 检查时间和时间间隔 查看是否有最新记录需要生成
            QList<Record> record_lst;
			{
				QMutexLocker lk(rtdb_mutex.data());
				res = m_pthis->QueryRecordByTime(rtdb, t, et, record_lst, &err);
			}
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

       
        int cnt = 2000;
        while(cnt > 0)
        {
            int single = 50;
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
