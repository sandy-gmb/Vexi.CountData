#include "config.hpp"
#include <QSettings>
#include <QTextCodec>
#include <QVariant>
#include <QFile>
#include <QTime>
#include <QSharedPointer>
#include <QStringList>
#include <logger.h>

#include "DataDef.hpp"

#define ConfigDir "Config"
#define ConfigFile "config.ini"

#define DefaultWordsFilePrefix "Words"

class Config::Impl
{
public:
    void Init()
    {
        QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //策略相关配置
        cfgStrategy.bStrategyMode = sets.value("strategy/mode", false).toBool();
        int size = sets.beginReadArray("strategy/sensorids");
        for(int i = 0; i < size; i++)
        {
            sets.setArrayIndex(i);
            cfgStrategy.lSensorIDs.append(sets.value("id", -1).toString());
        }
        sets.endArray();

        //系统配置
        cfgSystem.iTimeInterval_GeneRecord = sets.value("system/time_interval_generate_record", 5).toInt();
        cfgSystem.iTimeInterval_GetSrcData = sets.value("system/time_interval_get_src_data", 5).toInt();
        cfgSystem.iDaysDataOutDate = sets.value("system/data_outdate_days", 30).toInt();
        cfgSystem.iDefaultShow = sets.value("system/show_record_type", 1).toInt();
        cfgSystem.iLanguage = sets.value("system/language", 0).toInt();
        cfgSystem.sCodetsprefix = sets.value("system/code_file_prefix", "CodeTs").toString();
        //协议版本
        cfgSystem.iProtocolVersion = sets.value("system/protocol_version", 0).toInt();
        //时间间隔
        eTimeInterval = (ETimeInterval)sets.value("timeInterval/time_interval", 0).toInt();
        //班次
        size = sets.beginReadArray("shift/shift");
        QStringList shifttime;
        for(int i = 0; i < size; i++)
        {
            sets.setArrayIndex(i);
            shifttime.append(sets.value("time", "00:00:00").toString()) ;
        }
        sets.endArray();
        if(shifttime.isEmpty())
        {
            shifttime.append("00:00:00");
            sets.beginWriteArray("shift", 1);
            sets.setArrayIndex(0);
            sets.setValue("time", "00:00:00");
            sets.endArray();
            sets.sync();
        }
        ptrShift = QSharedPointer<Shift>(new Shift(shifttime));
        //调试配置
        cfgDebug.bReadSrcData = sets.value("debug/is_read_source_data", false).toBool();
        cfgDebug.bSaveSrcData = sets.value("debug/is_save_source_data", false).toBool();
        cfgDebug.iLogLevel = sets.value("debug/log_level", 2).toInt();
        sets.sync();
        ParseTranslationWords();
    }

    void Save()
    {
        QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //策略相关配置
        sets.setValue("strategy/mode", cfgStrategy.bStrategyMode);
        sets.beginWriteArray("strategy/sensorids", cfgStrategy.lSensorIDs.size());
        for(int i = 0; i < cfgStrategy.lSensorIDs.size(); i++)
        {
            sets.setArrayIndex(i);
            sets.setValue("id", cfgStrategy.lSensorIDs[i]);
        }
        sets.endArray();

        //系统配置
        sets.setValue("system/time_interval_generate_record", cfgSystem.iTimeInterval_GeneRecord);
        sets.setValue("system/time_interval_get_src_data", cfgSystem.iTimeInterval_GetSrcData);
        sets.setValue("system/data_outdate_days", cfgSystem.iDaysDataOutDate);
        sets.setValue("system/show_record_type", cfgSystem.iDefaultShow);
        sets.setValue("system/language", cfgSystem.iLanguage);
        sets.setValue("system/code_file_prefix", cfgSystem.sCodetsprefix);
        sets.setValue("system/protocol_version", cfgSystem.iProtocolVersion);
        //时间间隔
        sets.setValue("timeInterval/time_interval", (int)eTimeInterval);
        //班次
        sets.beginWriteArray("shift/shift", ptrShift->shifttime.size());
        for(int i = 0; i < ptrShift->shifttime.size(); i++)
        {
            sets.setArrayIndex(i);
            sets.setValue("time", ptrShift->shifttime[i].toString("hh:mm:ss"));
        }
        sets.endArray();
        //调试配置
        sets.setValue("debug/is_read_source_data", cfgDebug.bReadSrcData);
        sets.setValue("debug/is_save_source_data", cfgDebug.bSaveSrcData);
        sets.setValue("debug/log_level", cfgDebug.iLogLevel);
        sets.sync();
    }

    void SaveDefaultValue()
    {
        QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //策略相关配置
        sets.setValue("strategy/mode", false);
        sets.beginWriteArray("strategy/sensorids", 0);
        sets.endArray();

        //系统配置
        sets.setValue("system/time_interval_generate_record", 5);
        sets.setValue("system/time_interval_get_src_data", 1);
        sets.setValue("system/data_outdate_days", 30);
        sets.setValue("system/language", 0);
        sets.setValue("system/show_record_type", 1);
        sets.setValue("system/code_file_prefix", "CodeTs");
        sets.setValue("system/protocol_version", 0);
        //时间间隔
        sets.setValue("timeInterval/time_interval", 5);
        //班次
        sets.beginWriteArray("shift/shift", 1);
        sets.setArrayIndex(0);
        sets.setValue("time", "00:00:00");
        sets.endArray();
        //调试配置
        sets.setValue("debug/is_read_source_data", false);
        sets.setValue("debug/is_save_source_data", false);
        sets.setValue("debug/log_level", 2);
        sets.sync();
    }

    void ParseTranslationWords()
    {
        //根据语言找到对应的文件名
        QString lang = "_zh";
        if(cfgSystem.iLanguage == EL_English)
        {
            lang = "_en";
        }
        QString suffix = ".ini";
        QString prefix = cfgSystem.sCodetsprefix;
        if(prefix == "")
        {
            cfgSystem.sCodetsprefix = DefaultWordsFilePrefix;
        }
        QString filep = QString("%1/%2%3%4").arg(ConfigDir).arg(prefix).arg(lang).arg(suffix);
        moldwors.clear();
        sensorwors.clear();
        if(QFile::exists(filep))
        {//解析词条文件 
            QSettings sets(filep, QSettings::IniFormat);
            sets.setIniCodec(QTextCodec::codecForName("UTF-8"));

            sets.beginGroup("MoldWords");
            QStringList keys = sets.childKeys();
            foreach(QString key, keys)
            {
                moldwors.insert(key, sets.value(key).toString()  ) ;
            }
            sets.endGroup();
            sets.beginGroup("SensorWords");
            keys = sets.childKeys();
            foreach(QString key, keys)
            {
                sensorwors.insert(key, sets.value(key).toString() ) ;
            }
            sets.endGroup();
        }
    }

public:
    QSharedPointer<Shift> ptrShift;        //用来计算班次的对象

    StrategyConfig cfgStrategy;        //策略相关配置
    SystemConfig cfgSystem;            //系统相关配置
    ETimeInterval eTimeInterval;    //时间间隔 按时间间隔的时间段统计记录 1:60分钟 2:90分钟;3:120分钟 其他:30分钟 具体使用由DataCenter决定
    DebugConfg cfgDebug;            //调试相关配置

    //界面显示的词条翻译对象 需要根据语言解析对应翻译文件 用于界面显示
    QMap<QString, QString> moldwors;    //模板号翻译
    QMap<QString, QString> sensorwors;    //缺陷ID翻译
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
    cfg.eTimeInterval = pimpl->eTimeInterval;
    cfg.ptrShift = pimpl->ptrShift;
    cfg.strategy = pimpl->cfgStrategy;
    cfg.syscfg = pimpl->cfgSystem;
}

void Config::GetCoreConf(CoreConf& cfg)
{
    cfg.dbgcfg = pimpl->cfgDebug;
    cfg.syscfg = pimpl->cfgSystem;
}

void Config::GetSystemConf(SystemConfig& cfg)
{
    cfg = pimpl->cfgSystem;
}

void Config::GetAllConfig(AllConfig& cfg)
{
    cfg.dbgcfg = pimpl->cfgDebug;
    cfg.shiftlst = pimpl->ptrShift->getShiftTimeList();
    cfg.strategy = pimpl->cfgStrategy;
    cfg.syscfg = pimpl->cfgSystem;
    cfg.eTimeInterval = pimpl->eTimeInterval;
}

void Config::SetAllConfig(const AllConfig& cfg)
{
    bool debug, sys, strategy, timeinterval, shift, show, lang;
    debug = sys = strategy = timeinterval = shift = show = lang = true;
    
    QStringList changelog;
    changelog <<tr("Configuration Modify:");
    AllConfig _t;
    _t.dbgcfg = pimpl->cfgDebug;
    _t.syscfg = pimpl->cfgSystem;
    _t.shiftlst = pimpl->ptrShift->getShiftTimeList();
    _t.eTimeInterval = pimpl->eTimeInterval;
    _t.strategy = pimpl->cfgStrategy;

    if(cfg.dbgcfg != pimpl->cfgDebug)
    {
        pimpl->cfgDebug = cfg.dbgcfg;
        debug = false;
    }
    if(cfg.syscfg != pimpl->cfgSystem)
    {
        if(cfg.syscfg.iDefaultShow != pimpl->cfgSystem.iDefaultShow)
        {
            show = false;
        }
        if(cfg.syscfg.iLanguage != pimpl->cfgSystem.iLanguage)
        {
            lang = false;
        }

        pimpl->cfgSystem = cfg.syscfg;
        sys = false;
    }
    if(cfg.strategy != pimpl->cfgStrategy)
    {
        pimpl->cfgStrategy = cfg.strategy;
        strategy = false;
    }
    if( *pimpl->ptrShift != cfg.shiftlst)
    {
        pimpl->ptrShift->reset(cfg.shiftlst);
        shift = false;
    }
    if(cfg.eTimeInterval != pimpl->eTimeInterval)
    {
        pimpl->eTimeInterval = cfg.eTimeInterval;
        timeinterval = false;
    }
    ELOGI("Configuration Modify %s", qPrintable(_t.changeLog(cfg)));

    if( !debug || !sys || !strategy || !timeinterval || !shift)
    {
        pimpl->Save();
    }

    if(!timeinterval || !shift)
    {
        emit RecordConfigChanged();
    }
    if(!debug || !sys)
    {
        CoreConf cfg;
        GetCoreConf(cfg);
        emit CoreConfChange(cfg);
    }

    /*if( !sys || !strategy || !timeinterval || !shift)
    {
        DataCenterConf cfg;
        GetDataCenterConf(cfg);
        emit DataConfChange(cfg);
    }*/

    if(!show)
    {
        emit RecordShowChanged(cfg.syscfg.iDefaultShow);
    }

    if(!lang)
    {
        pimpl->ParseTranslationWords();
        emit LanguageChanged(cfg.syscfg.iLanguage);
    }
    
}

int Config::GetConfigLogLevel()
{
    return pimpl->cfgDebug.iLogLevel;
}

void Config::GetWordsTranslation(QMap<QString, QString>& moldwors, QMap<QString, QString>& sensorwors)
{
    moldwors = pimpl->moldwors;
    sensorwors = pimpl->sensorwors;
}

int Config::GetTimeInterval()
{
    return (int)pimpl->eTimeInterval;
}

