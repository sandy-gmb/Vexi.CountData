#ifndef CONFIG_DEFINE_HH_
#define CONFIG_DEFINE_HH_

#pragma once

#include <QList>

class DataCenterConf
{
public:
    //策略数据
    bool bStrategyMode;                 //策略模式 true表示 白名单 false表示黑名单
    QList<int> lSensorIDs;               //策略影响的缺陷ID列表
    int iTimeInterval_GeneRecord;       //时间间隔,用于在多长时间生成一条统计记录 1:60分钟 2:90分钟;3:120分钟 其他:30分钟
    int iDaysDataOutDate;               //原始数据有效天数 单位:天,超过此时间,数据库会删除
    bool bIsLogEveryData;               //是否日志记录所有数据
};

//int iTimeInterval_GetSrcData;       //获取原始数据的时间间隔 单位:秒

//int iLanguage;                       //语言 0:中文 1:英文 涉及到界面显示语言和使用的词条对应文件名
//QString sCodetsprefix;              //词条文件前缀 如配置为Error,语言配置为0,则实际使用文件为Error_zh.ini

#endif//CONFIG_DEFINE_HH_
