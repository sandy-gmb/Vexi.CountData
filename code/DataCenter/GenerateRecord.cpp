	#include "GenerateRecord.h"
#include "DBOperation.h"

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

bool GenerateRecord::processGenerTimeIntervalRecord()
{
	/***************************************************************************************************
	* 函数实现:
	*
	* 1 检查记录数据库的主表,得到 最新的日志截止时间 QDateTime t 有可能没有
	* 2 根据t (没有t,则以实时记录的第一条的起始时间为准),计算需要添加的记录间隔,
	* 3 检查实时数据数据库,得到所有在t之后的数据,如果没有t,则获取所有
	* 4 根据时间间隔 和 原始数据,统计得到记录数据
	* 5 根据统计记录数据,生成每一条记录,放到记录数据库中
	*
	****************************************************************************************************/
	bool res = false;
	QString err;
	do 
	{
		QDateTime t;
		//1 检查记录数据库的主表,得到 最新的日志截止时间 QDateTime t,如果获取失败,则从原始记录表获取第一条记录的起始时间
		res = m_pthis->GetLatestRecordEndTime(t, &err);
		if(!res)
		{
			ELOGD(qPrintable(err));
			break;
		}

		//2 根据截止时间 和时间间隔 计算需要生成记录的时间间隔
		QList<QDateTime> timelst;
		ELOGD("%s", t.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
		CalcGenerateRecordTime(t, eti, timelst);
		if(timelst.isEmpty())
		{
			m_pthis->DeleteRecordAfterTime( EOperDB_TimeIntervalDB, QDateTime::currentDateTime());
			break;
		}
		if(!runflg){
			break;
		}
		QDateTime et = timelst.last();

		//3 检查时间和时间间隔 查看是否有最新记录需要生成
		QList<Record> record_lst;
		{
			res = m_pthis->QueryRecordByTime(EOperDB_RuntimeDB, t, et, record_lst, &err);
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
		res = m_pthis->SaveRecordList(EOperDB_TimeIntervalDB, statistic_record_lst, &err);

		if(timelst.size() == 5)
		{
			continue;
		}
	} while (false);
	return res;
}

bool GenerateRecord::processGenerateShiftRecord()
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
	QString err;
	do 
	{
		QDateTime t;
		//1 检查记录数据库的主表,得到 最新的日志截止时间 QDateTime t,如果获取失败,则从原始记录表获取第一条记录的起始时间
		res = m_pthis->GetLastestShiftRecordEndTime(t, &err);
		if(!res)
		{
			ELOGD(qPrintable(err));
			break;
		}

		//2 根据截止时间 和时间间隔 计算需要生成记录的时间间隔
		QList<QDateTime> timelst;
		ELOGD("Last ShiftTime:%s", t.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
		CalcGenerateShiftRecordTime(t, timelst);
		if(timelst.isEmpty())
		{
			m_pthis->DeleteRecordAfterTime( EOperDB_ShiftDB, QDateTime::currentDateTime());
			break;
		}
		if(!runflg){
			break;
		}
		t = timelst.first();
		QDateTime et = timelst.last();

		//3 检查时间和时间间隔 查看是否有最新记录需要生成
		QList<Record> record_lst;
		{
			res = m_pthis->QueryRecordByTime(EOperDB_RuntimeDB, t, et, record_lst, &err);
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
		res = StatisticsRecord(timelst, record_lst, statistic_record_lst, &err, true);
		if(!runflg){
			break;
		}
		//5 生成记录
		res = m_pthis->SaveRecordList( EOperDB_ShiftDB, statistic_record_lst, &err);
		if(timelst.size() == 5)
		{
			continue;
		}
	} while (false);
	return res;
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
		if(lst.size() >= 5)
		{
		lst.append(st);
		return;
		}
	}

	if(!lst.isEmpty())
	{
		//最后一个是截止时间
		lst.push_back(st);
	}
}

bool GenerateRecord::StatisticsRecord(QList<QDateTime>& tlst, QList<Record>& lst, QList<Record>& newlst, QString* err, bool isshiftrecord /*= false*/)
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
		if(isshiftrecord)
		{
			shift->getShiftDateShift(r.dt_start, r.date, r.shift);
		}
		newlst.push_back(r);
		if(j == lst.size())
		{
			break;
		}
	}
	return true;
}

void GenerateRecord::CalcGenerateShiftRecordTime(QDateTime start, QList<QDateTime>& lst)
{
	start = shift->getLastShiftTime(start);
	QDateTime t = QDateTime::currentDateTime();
	t = shift->getNextShiftTime(t);
	QStringList tttt;
	//根据班次起始时间和班次定义 截止时间是当前班次的截止时间点 计算所有需要查询记录的时间
	while(start < t )
	{
		tttt.append(start.toString());
		lst.append(start);
		start = shift->getNextShiftTime(start);

		if(lst.size() >= 5)
		{
			lst.append(start);
			return;
		}
	}
	if(!lst.isEmpty())
	{
		lst.append(t);
	}
}

GenerateRecord::GenerateRecord( QObject *parent /*= nullptr*/ )
	: QObject(nullptr)
	, runflg(true)
{
	m_pthis = (DBOperation*)parent;

	eti = ETI_30_Min;
	//默认30天过期
	cfgSystem.iDaysDataOutDate = 30;

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
				m_pthis->DeleteOutdatedData(cfgSystem.iDaysDataOutDate);
				dellast = QDateTime::currentDateTime();
			}
			if(!runflg){
				break;
			}
			res = processGenerTimeIntervalRecord();
			if(!res)
			{
				break;
			}
			if(!runflg){
				break;
			}

			res = processGenerateShiftRecord();
			if(!res)
			{
				break;
			}
		} while (false);

		if(!runflg){
			break;
		}
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
