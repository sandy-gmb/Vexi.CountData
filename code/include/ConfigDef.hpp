#ifndef CONFIG_DEFINE_HH_
#define CONFIG_DEFINE_HH_

#pragma once

#include <QList>
#include "DataDef.hpp"
#include <QStringList>

class DebugConfg
{
public:
	bool operator!=(const DebugConfg& d)const
	{
		return bSaveSrcData != d.bSaveSrcData ||
			bReadSrcData != d.bReadSrcData ||
			iLogLevel != d.iLogLevel;
	}
	QString changeLog(const DebugConfg& d)const
	{
		QString t = "";
		if(bSaveSrcData != d.bSaveSrcData)
		{
			t += QString("bSaveSrcData:%1 -> %2\n").arg(bSaveSrcData).arg(d.bSaveSrcData);
		}
		if(bReadSrcData != d.bReadSrcData)
		{
			t += QString("bReadSrcData:%1 -> %2\n").arg(bReadSrcData).arg(d.bReadSrcData);
		}
		if(iLogLevel != d.iLogLevel)
		{
			t += QString("iLogLevel:%1 -> %2\n").arg(iLogLevel).arg(d.iLogLevel);
		}
		return t;
	}

	bool bReadSrcData;					//�Ƿ��ȡԴ�����ļ�
	bool bSaveSrcData;					//�Ƿ񱣴�Դ���ݵ������ļ�
	int iLogLevel;						//��־����
};

class StrategyConfig
{
public:
	bool operator!=(const StrategyConfig& d)const
	{
		return bStrategyMode != d.bStrategyMode ||
			lSensorIDs != d.lSensorIDs;
	}

	QString changeLog(const StrategyConfig& d)const
	{
		QString t = "";
		if(bStrategyMode != d.bStrategyMode)
		{
			t += QString("bStrategyMode:%1 -> %2\n").arg(bStrategyMode).arg(d.bStrategyMode);
		}
		if(lSensorIDs != d.lSensorIDs)
		{
			t += QString("SensorIDs:");
			foreach(int id, lSensorIDs)
			{
				t += QString("%1,").arg(id);
			}
			t += QString(" -> ");
			foreach(int id, d.lSensorIDs)
			{
				t += QString("%1,").arg(id);
			}
			t += QString("\n");
		}
		return t;
	}

	bool bStrategyMode;                 //����ģʽ true��ʾ ������ false��ʾ������
	QList<int> lSensorIDs;               //����Ӱ���ȱ��ID�б�
};

class SystemConfig
{
public:
	bool operator!=(const SystemConfig& d)const
	{
		return iTimeInterval_GeneRecord != d.iTimeInterval_GeneRecord ||
			iTimeInterval_GetSrcData != d.iTimeInterval_GetSrcData ||
			iDaysDataOutDate != d.iDaysDataOutDate ||
			iDefaultShow != d.iDefaultShow ||
			iLanguage != d.iLanguage ||
			sCodetsprefix != d.sCodetsprefix;
	}

	QString changeLog(const SystemConfig& d)const
	{
		QString t = "";
		if(iTimeInterval_GeneRecord != d.iTimeInterval_GeneRecord)
		{
			t += QString("iTimeInterval_GeneRecord:%1 -> %2\n").arg(iTimeInterval_GeneRecord).arg(d.iTimeInterval_GeneRecord);
		}
		if(iTimeInterval_GetSrcData != d.iTimeInterval_GetSrcData)
		{
			t += QString("iTimeInterval_GetSrcData:%1 -> %2\n").arg(iTimeInterval_GetSrcData).arg(d.iTimeInterval_GetSrcData);
		}
		if(iDaysDataOutDate != d.iDaysDataOutDate)
		{
			t += QString("iDaysDataOutDate:%1 -> %2\n").arg(iDaysDataOutDate).arg(d.iDaysDataOutDate);
		}
		if(iDefaultShow != d.iDefaultShow)
		{
			t += QString("iDefaultShow:%1 -> %2\n").arg(iDefaultShow).arg(d.iDefaultShow);
		}
		if(iLanguage != d.iLanguage)
		{
			t += QString("iLanguage:%1 -> %2\n").arg(iLanguage).arg(d.iLanguage);
		}
		if(sCodetsprefix != d.sCodetsprefix)
		{
			t += QString("sCodetsprefix:%1 -> %2\n").arg(sCodetsprefix).arg(d.sCodetsprefix);
		}
		return t;
	}

	int iTimeInterval_GeneRecord;       //ʱ����,�����ڼ���೤ʱ��ˢ��ͳ�Ƽ�¼,��λ��
	int iTimeInterval_GetSrcData;       //��ȡԭʼ���ݵ�ʱ���� ��λ:��
	int iDaysDataOutDate;               //ԭʼ������Ч���� ��λ:��,������ʱ��,���ݿ��ɾ��
	int iDefaultShow;					//����Ĭ����ʾ 0:��ʱ������ʾ 1:�������ʾ ERecordType

	int iLanguage;                       //���� 0:���� 1:Ӣ�� �漰��������ʾ���Ժ�ʹ�õĴ�����Ӧ�ļ���
	QString sCodetsprefix;              //�����ļ�ǰ׺ ������ΪError,��������Ϊ0,��ʵ��ʹ���ļ�ΪError_zh.ini
};

class DataCenterConf
{
public:
   StrategyConfig strategy;
   SystemConfig syscfg;
   QSharedPointer<Shift> ptrShift;
   ETimeInterval eTimeInterval;
};

class CoreConf
{
public:
	SystemConfig syscfg;
	DebugConfg dbgcfg;
};

class AllConfig
{
public:
	bool operator!=(const AllConfig& t)const
	{
		return strategy != t.strategy ||
			syscfg != t.syscfg ||
			dbgcfg != t.dbgcfg ||
			shiftlst != t.shiftlst ||
			eTimeInterval != t.eTimeInterval;
	}

	QString changeLog(const AllConfig& d)const
	{
		QString t = "";
		if(strategy != d.strategy)
		{
			t += QString("strategy:%1").arg(strategy.changeLog(d.strategy));
		}
		if(syscfg != d.syscfg)
		{
			t += QString("syscfg:%1").arg(syscfg.changeLog(d.syscfg));
		}
		if(dbgcfg != d.dbgcfg)
		{
			t += QString("dbgcfg:%1").arg(dbgcfg.changeLog(d.dbgcfg));
		}
		if(shiftlst != d.shiftlst)
		{
			t += QString("shiftlst:%1 -> %2\n").arg(shiftlst.join(",")).arg(d.shiftlst.join(","));
		}
		if(eTimeInterval != d.eTimeInterval)
		{
			t += QString("eTimeInterval:%1 -> %2\n").arg(eTimeInterval).arg(d.eTimeInterval);
		}
		return t;
	}

	StrategyConfig strategy;
	SystemConfig syscfg;
	DebugConfg dbgcfg;
	QStringList shiftlst;
	ETimeInterval eTimeInterval;

};

//int iTimeInterval_GetSrcData;       //��ȡԭʼ���ݵ�ʱ���� ��λ:��

//int iLanguage;                       //���� 0:���� 1:Ӣ�� �漰��������ʾ���Ժ�ʹ�õĴ�����Ӧ�ļ���
//QString sCodetsprefix;              //�����ļ�ǰ׺ ������ΪError,��������Ϊ0,��ʵ��ʹ���ļ�ΪError_zh.ini

#endif//CONFIG_DEFINE_HH_
