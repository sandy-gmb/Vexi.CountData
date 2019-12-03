#include "config.hpp"
#include <QSettings>
#include <QTextCodec>
#include <QVariant>

#define ConfigDir "Config"
#define ConfigFile "config.ini"

#define DefaultWordsFilePrefix "Words"

enum ELanguage
{
    EL_Chinese  = 0,         //中文
    EL_English  = 1,         //英文
};

class Config::Impl
{
public:
    void Init()
    {
        QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //策略相关配置
        bStrategyMode = sets.value("strategy/mode", true).toBool();
        lSensorIDs.clear();
        int size = sets.beginReadArray("strategy/sensorids");
        for(int i = 0; i < size; i++)
        {
            sets.setArrayIndex(i);
            int id = sets.value(QString::number(i), -1).toInt();
            lSensorIDs.push_back(id);
        }
        sets.endArray();
        bIsLogEveryData = sets.value("strategy/islogall", false).toBool();
        //时间间隔等配置 system
        iTimeInterval_GeneRecord = sets.value("system/time_interval_generate_record", 0).toInt();
        iTimeInterval_GetSrcData = sets.value("system/time_interval_get_src_data", 60).toInt();
        iDaysDataOutDate = sets.value("system/data_outdate_days", 30).toInt();

        //系统语言及词条文件相关配置
        iLanguage = sets.value("system/language", 0).toInt();
        sCodetsprefix = sets.value("system/code_file_prefix", "Words").toString();
        
    }

    void Save()
    {
        QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //策略相关配置
        sets.setValue("strategy/mode", bStrategyMode);
        if(lSensorIDs.size() == 0)
        {
            lSensorIDs.append(10);
            lSensorIDs.append(34);
        }
        sets.beginWriteArray("strategy/sensorids", lSensorIDs.size());
        for(int i = 0; i < lSensorIDs.size(); i++)
        {
            sets.setArrayIndex(i);
            sets.setValue(QString::number(i), lSensorIDs[i]);
        }
        sets.endArray();
        sets.setValue("strategy/islogall", bIsLogEveryData);

        //时间间隔等配置 system
        sets.setValue("system/time_interval_generate_record", iTimeInterval_GeneRecord);
        sets.setValue("system/time_interval_get_src_data", iTimeInterval_GetSrcData);
        sets.setValue("system/data_outdate_days", iDaysDataOutDate);

        //系统语言及词条文件相关配置
        sets.setValue("system/language", iLanguage);
        sets.setValue("system/code_file_prefix", sCodetsprefix);
        sets.sync();
    }

public:
    bool bStrategyMode;                 //策略模式 true表示 白名单 false表示黑名单
    QList<int> lSensorIDs;               //策略影响的缺陷ID列表
    int iTimeInterval_GeneRecord;       //时间间隔,用于在多长时间生成一条统计记录 1:60分钟 2:90分钟;3:120分钟 其他:30分钟 具体使用由DataCenter决定
    int iDaysDataOutDate;               //原始数据有效天数 单位:天,超过此时间,数据库会删除
    bool bIsLogEveryData;               //是否日志记录所有数据

    int iTimeInterval_GetSrcData;       //获取原始数据的时间间隔 单位:秒

    int iLanguage;                       //语言 0:中文 1:英文 涉及到界面显示语言和使用的词条对应文件名
    QString sCodetsprefix;              //词条文件前缀 如配置为Error,语言配置为0,则实际使用文件为Error_zh.ini
};


//内部实质管理一个文件 另外词条对应系统 使用的文件为一系列 

Config::Config(QObject* parent)
    : QObject(parent)
    , pimpl(new Impl)
{
    pimpl->Init();
}

Config::~Config()
{
    //pimpl->Save();
    delete pimpl;
}

void Config::GetDataCenterConf( DataCenterConf& cfg )
{
    cfg.bStrategyMode = pimpl->bStrategyMode;
    cfg.lSensorIDs = pimpl->lSensorIDs;
    cfg.iTimeInterval_GeneRecord = pimpl->iTimeInterval_GeneRecord;
    cfg.iDaysDataOutDate = pimpl->iDaysDataOutDate;
    cfg.bIsLogEveryData = pimpl->bIsLogEveryData;
}

int Config::GetSoftwareLanguage()
{
    return pimpl->iLanguage;
}

QString Config::GetWordsTranslationFilePath()
{
    QString lang = "_zh";
    if(pimpl->iLanguage == EL_English)
    {
        lang = "_en";
    }
    QString suffix = ".ini";
    QString prefix = pimpl->sCodetsprefix;
    if(pimpl->sCodetsprefix == "")
    {
        pimpl->sCodetsprefix = DefaultWordsFilePrefix;
    }
    return QString("%1/%2%3%4").arg(ConfigDir).arg(prefix).arg(lang).arg(suffix);
}

int Config::GetTimeOfObtainSrcData()
{
    return pimpl->iTimeInterval_GetSrcData;
}

void Config::SetGenerateRecordTimeInterval( int ti )
{
    pimpl->iTimeInterval_GeneRecord = ti;
    pimpl->Save();
}


