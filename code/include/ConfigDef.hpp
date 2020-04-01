#ifndef CONFIG_DEFINE_HH_
#define CONFIG_DEFINE_HH_

#pragma once

#include <QList>
#include "DataDef.hpp"

class DebugConfg
{
public:
	bool operator!=(const DebugConfg& d)const
	{
		return bSaveSrcData != d.bSaveSrcData &&
			bReadSrcData != d.bReadSrcData &&
			iLogLevel != d.iLogLevel;
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
		return bStrategyMode != d.bStrategyMode &&
			lSensorIDs != d.lSensorIDs;
	}

	bool bStrategyMode;                 //����ģʽ true��ʾ ������ false��ʾ������
	QList<int> lSensorIDs;               //����Ӱ���ȱ��ID�б�
};

class SystemConfig
{
public:
	bool operator!=(const SystemConfig& d)const
	{
		return iTimeInterval_GeneRecord != d.iTimeInterval_GeneRecord &&
			iTimeInterval_GetSrcData != d.iTimeInterval_GetSrcData &&
			iDaysDataOutDate != d.iDaysDataOutDate &&
			iDefaultShow != d.iDefaultShow &&
			iLanguage != d.iLanguage &&
			sCodetsprefix != d.sCodetsprefix;
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
		return strategy != t.strategy &&
			syscfg != t.syscfg &&
			dbgcfg != t.dbgcfg &&
			shiftlst != t.shiftlst &&
			eTimeInterval != t.eTimeInterval;
	}


	StrategyConfig strategy;
	SystemConfig syscfg;
	DebugConfg dbgcfg;
	QList<QString> shiftlst;
	ETimeInterval eTimeInterval;

};

//int iTimeInterval_GetSrcData;       //��ȡԭʼ���ݵ�ʱ���� ��λ:��

//int iLanguage;                       //���� 0:���� 1:Ӣ�� �漰��������ʾ���Ժ�ʹ�õĴ�����Ӧ�ļ���
//QString sCodetsprefix;              //�����ļ�ǰ׺ ������ΪError,��������Ϊ0,��ʵ��ʹ���ļ�ΪError_zh.ini

#endif//CONFIG_DEFINE_HH_
