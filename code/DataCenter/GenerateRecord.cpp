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

const int iEveryTimeMaxDuration = 5;

bool GenerateRecord::processGenerTimeIntervalRecord()
{
	/***************************************************************************************************
	* ����ʵ��:
	*
	* 1 ����¼���ݿ������,�õ� ���µ���־��ֹʱ�� QDateTime t �п���û��
	* 2 ����t (û��t,����ʵʱ��¼�ĵ�һ������ʼʱ��Ϊ׼),������Ҫ��ӵļ�¼���,
	* 3 ���ʵʱ�������ݿ�,�õ�������t֮�������,���û��t,���ȡ����
	* 4 ����ʱ���� �� ԭʼ����,ͳ�Ƶõ���¼����
	* 5 ����ͳ�Ƽ�¼����,����ÿһ����¼,�ŵ���¼���ݿ���
	*
	****************************************************************************************************/
	bool res = false;
	QString err;
	do 
	{
		QDateTime t;
		//1 ����¼���ݿ������,�õ� ���µ���־��ֹʱ�� QDateTime t,�����ȡʧ��,���ԭʼ��¼���ȡ��һ����¼����ʼʱ��
		res = m_pthis->GetLatestRecordEndTime(t, &err);
		if(!res)
		{
			ELOGD(qPrintable(err));
			break;
		}

		//2 ���ݽ�ֹʱ�� ��ʱ���� ������Ҫ���ɼ�¼��ʱ����
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

		//3 ���ʱ���ʱ���� �鿴�Ƿ������¼�¼��Ҫ����
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

		//4 �������������б�ͳ��ÿ�����������
		QList<Record> statistic_record_lst;
		res = StatisticsRecord(timelst, record_lst, statistic_record_lst, &err);
		if(!runflg){
			break;
		}
		//5 ���ɼ�¼
		res = m_pthis->SaveRecordList(EOperDB_TimeIntervalDB, statistic_record_lst, &err);

		if(timelst.size() == iEveryTimeMaxDuration)
		{
			continue;
		}
	} while (false);
	return res;
}

bool GenerateRecord::processGenerateShiftRecord()
{
	/***************************************************************************************************
	* ����ʵ��:
	*  ��ʱɾ������ԭʼ��¼
	*
	* 1 ����¼���ݿ������,�õ� ���µ���־��ֹʱ�� QDateTime t �п���û��
	* 2 ����t (û��t,����ʵʱ��¼�ĵ�һ������ʼʱ��Ϊ׼),������Ҫ��ӵļ�¼���,
	* 3 ���ʵʱ�������ݿ�,�õ�������t֮�������,���û��t,���ȡ����
	* 4 ����ʱ���� �� ԭʼ����,ͳ�Ƶõ���¼����
	* 5 ����ͳ�Ƽ�¼����,����ÿһ����¼,�ŵ���¼���ݿ���
	*
	****************************************************************************************************/
	bool res = false;
	QString err;
	do 
	{
		QDateTime t;
		//1 ����¼���ݿ������,�õ� ���µ���־��ֹʱ�� QDateTime t,�����ȡʧ��,���ԭʼ��¼���ȡ��һ����¼����ʼʱ��
		res = m_pthis->GetLastestShiftRecordEndTime(t, &err);
		if(!res)
		{
			ELOGD(qPrintable(err));
			break;
		}

		//2 ���ݽ�ֹʱ�� ��ʱ���� ������Ҫ���ɼ�¼��ʱ����
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

		//3 ���ʱ���ʱ���� �鿴�Ƿ������¼�¼��Ҫ����
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

		//4 �������������б�ͳ��ÿ�����������
		QList<Record> statistic_record_lst;
		res = StatisticsRecord(timelst, record_lst, statistic_record_lst, &err, true);
		if(!runflg){
			break;
		}
		//5 ���ɼ�¼
		res = m_pthis->SaveRecordList( EOperDB_ShiftDB, statistic_record_lst, &err);
		if(timelst.size() == iEveryTimeMaxDuration)
		{
			continue;
		}
	} while (false);
	return res;
}

/**
* @brief  :  CalcGenerateRecordTime ���ص�����ʼʱ��
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
	//�����һ��ʱ��������ʼʱ��
	QDateTime t(start.date(), QTime(0,0));
	//���賿12�㿪ʼ������ʼʱ�����ڵ�����,
	int passd = t.secsTo(start)/60/ti;
	QDateTime st = t;
	st = st.addSecs(passd*ti*60);
	QStringList tslst;

	while(st.secsTo(end) > ti*60 )
	{//������Ҫ���ɵļ�¼���� ��Ҫ���ɵļ�¼������Ҫ������ǰ������
		tslst.push_back(st.toString());

		lst.push_back(st);
		st = st.addSecs(ti*60);
		if(lst.size() >= iEveryTimeMaxDuration)
		{
		lst.append(st);
		return;
		}
	}

	if(!lst.isEmpty())
	{
		//���һ���ǽ�ֹʱ��
		lst.push_back(st);
	}
}

bool GenerateRecord::StatisticsRecord(QList<QDateTime>& tlst, QList<Record>& lst, QList<Record>& newlst, QString* err, bool isshiftrecord /*= false*/)
{//�����������ݲ���Ϊ��
	if(tlst.size() < 2 || lst.isEmpty())
	{//ʱ����Ϊ�� ����Ԫ����Ϊ��
		//����ʱ��ֽ������������(��ʼ+����),����ж�С��2
		SAFE_SET(err, QObject::tr("the data isn't enough"));
		return false;
	}
	/**************************************************
	* 1 ͬʱ����ʱ��ֽ������ݽڵ�
	* ע:Ĭ�������������(ʱ������ ͳ�Ƽ�¼ʱ������),
	***************************************************/

	int j = 0;
	for(int i = 0; i < tlst.size()-1; i++)
	{//����ʱ��ֽ��
		Record r;
		r.dt_start = tlst[i];
		r.dt_end = tlst[i+1];
		//ͳ�Ƹ�ʱ��ε�����
		for(; j < lst.size();j++)
		{
			QString t1s = r.dt_start.toString();
			QString t1e = r.dt_end.toString();
			QString t2s = lst[j].dt_start.toString();
			QString t2e = lst[j].dt_end.toString();
			if(lst[j].dt_end <= r.dt_end && lst[j].dt_end >= r.dt_start)
			{//ԭ��ֻ���ж���ֹʱ���ڱ�ѡʱ���֮ǰ
				r.id = lst[j].id;
				r.MergeData(lst[j]);
			}
			else if(lst[j].dt_end > r.dt_end)
			{//������һ��ʱ��ε�����,���˳�,��֯��һ��ͳ�Ƽ�¼
				break;
			}
		}
		if(isshiftrecord)
		{
			//ȡʱ����м��ʱ��ȡ��ȡ������ںͰ�ε��ж�,��ֹ�߽��ж�����
			QDateTime t = r.dt_start.addSecs(r.dt_start.secsTo(r.dt_end) / 2);
			shift->getShiftDateShift(t, r.date, r.shift);
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
	//���ݰ����ʼʱ��Ͱ�ζ��� ��ֹʱ���ǵ�ǰ��εĽ�ֹʱ��� ����������Ҫ��ѯ��¼��ʱ��
	while(start < t )
	{
		tttt.append(start.toString());
		lst.append(start);
		start = shift->getNextShiftTime(start);

		if(lst.size() >= iEveryTimeMaxDuration)
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
	//Ĭ��30�����
	cfgSystem.iDaysDataOutDate = 30;

}

void GenerateRecord::work()
{
	/***************************************************************************************************
	* ����ʵ��:
	*  ��ʱɾ������ԭʼ��¼
	*
	* 1 ����¼���ݿ������,�õ� ���µ���־��ֹʱ�� QDateTime t �п���û��
	* 2 ����t (û��t,����ʵʱ��¼�ĵ�һ������ʼʱ��Ϊ׼),������Ҫ��ӵļ�¼���,
	* 3 ���ʵʱ�������ݿ�,�õ�������t֮�������,���û��t,���ȡ����
	* 4 ����ʱ���� �� ԭʼ����,ͳ�Ƶõ���¼����
	* 5 ����ͳ�Ƽ�¼����,����ÿһ����¼,�ŵ���¼���ݿ���
	*
	****************************************************************************************************/
	bool res = false;

	QDateTime dellast = QDateTime::currentDateTime();
	dellast = dellast.addDays(-2);
	QString err;
	while (runflg) 
	{//���ɼ�¼

		do 
		{
			//��ʱɾ����¼ ÿ��ִ��һ��,
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
			//���ɼ�¼�ܺ�ʱ,����ʵʱ��Ҫ�󲻸�,��˿�������Ϊ5 sec
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
