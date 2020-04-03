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

	bool bReadSrcData;					//是否读取源数据文件
	bool bSaveSrcData;					//是否保存源数据到数据文件
	int iLogLevel;						//日志级别
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

	bool bStrategyMode;                 //策略模式 true表示 白名单 false表示黑名单
	QList<int> lSensorIDs;               //策略影响的缺陷ID列表
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

	int iTimeInterval_GeneRecord;       //时间间隔,用于在间隔多长时间刷新统计记录,单位秒
	int iTimeInterval_GetSrcData;       //获取原始数据的时间间隔 单位:秒
	int iDaysDataOutDate;               //原始数据有效天数 单位:天,超过此时间,数据库会删除
	int iDefaultShow;					//界面默认显示 0:按时间间隔显示 1:按班次显示 ERecordType

	int iLanguage;                       //语言 0:中文 1:英文 涉及到界面显示语言和使用的词条对应文件名
	QString sCodetsprefix;              //词条文件前缀 如配置为Error,语言配置为0,则实际使用文件为Error_zh.ini
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

//int iTimeInterval_GetSrcData;       //获取原始数据的时间间隔 单位:秒

//int iLanguage;                       //语言 0:中文 1:英文 涉及到界面显示语言和使用的词条对应文件名
//QString sCodetsprefix;              //词条文件前缀 如配置为Error,语言配置为0,则实际使用文件为Error_zh.ini

#endif//CONFIG_DEFINE_HH_
