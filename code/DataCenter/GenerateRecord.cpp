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
			if(m_pthis->GetLastestRecordEndTime(db, t, err))
			{
				return true;
			}
			//
		}
		{
			QMutexLocker lk(rtdb_mutex.data());
			//1 �Ӽ�¼��� ��ȡ���¼�¼�Ľ�ֹʱ��
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
    }

    if(!lst.isEmpty())
    {
        //���һ���ǽ�ֹʱ��
        lst.push_back(st);
	}
}

bool GenerateRecord::StatisticsRecord(QList<QDateTime>& tlst, QList<Record>& lst, QList<Record>& newlst, QString* err)
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

        //�����ݿ�
        if(!db->open())
        {
			SAFE_SET(err, QString(QObject::tr("Open Record database failure,the erro is %1").arg(db->lastError().text()))) ;
            return false;
        }
        if(!db->transaction())
        {//��������ʧ��
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+db->lastError().text())) ;
            db->close();
            return false;
        }

        foreach(Record data, newlst)
        {//�����������
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
    //��ǰʱ��
    QString t = QDateTime::currentDateTime().addDays(outdatedays*-1).toString("yyyy-MM-dd hh:mm:ss");
    //��������� ���ֱ��ɾ���������� ����ɾ���������
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
	//��ǰʱ��
	QString t = time.toString("yyyy-MM-dd hh:mm:ss");
	//QString et = time.addSecs(ETimeInterval2Min(eti)*60).toString("yyyy-MM-dd hh:mm:ss");
	//��������� ���ֱ��ɾ���������� ����ɾ���������
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
    //Ĭ��30�����
    outdatedays = 30;

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
                DeleteOutdatedData(outdatedays);
                dellast = QDateTime::currentDateTime();
            }
            if(!runflg){
                break;
            }
            QDateTime t;
            //1 ����¼���ݿ������,�õ� ���µ���־��ֹʱ�� QDateTime t,�����ȡʧ��,���ԭʼ��¼���ȡ��һ����¼����ʼʱ��
            res = GetLastestRecordEndTime(t, &err);
            if(!res)
            {
                ELOGD(qPrintable(err));
            }

            //2 ���ݽ�ֹʱ�� ��ʱ���� ������Ҫ���ɼ�¼��ʱ����
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

            //3 ���ʱ���ʱ���� �鿴�Ƿ������¼�¼��Ҫ����
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

            //4 �������������б�ͳ��ÿ�����������
            QList<Record> statistic_record_lst;
            res = StatisticsRecord(timelst, record_lst, statistic_record_lst, &err);
            if(!runflg){
                break;
            }
            //5 ���ɼ�¼
            res = GenerateRecordToDB(statistic_record_lst, &err);

        } while (false);

       
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
