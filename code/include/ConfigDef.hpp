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

	bool bReadSrcData;					//是否读取源数据文件
	bool bSaveSrcData;					//是否保存源数据到数据文件
	int iLogLevel;						//日志级别
};

class StrategyConfig
{
public:
	bool operator!=(const StrategyConfig& d)const
	{
		return bStrategyMode != d.bStrategyMode &&
			lSensorIDs != d.lSensorIDs;
	}

	bool bStrategyMode;                 //策略模式 true表示 白名单 false表示黑名单
	QList<int> lSensorIDs;               //策略影响的缺陷ID列表
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

//int iTimeInterval_GetSrcData;       //获取原始数据的时间间隔 单位:秒

//int iLanguage;                       //语言 0:中文 1:英文 涉及到界面显示语言和使用的词条对应文件名
//QString sCodetsprefix;              //词条文件前缀 如配置为Error,语言配置为0,则实际使用文件为Error_zh.ini

#endif//CONFIG_DEFINE_HH_
