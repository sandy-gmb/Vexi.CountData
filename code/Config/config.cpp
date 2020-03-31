#include "config.hpp"
#include <QSettings>
#include <QTextCodec>
#include <QVariant>
#include <QFile>
#include <QTime>

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
		QString filep = QString("%1/%2").arg(ConfigDir).arg(ConfigFile);
		if(!QFile::exists(filep))
		{
			SaveDefaultValue();
		}
        QSettings sets(filep, QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //策略相关配置
        bStrategyMode = sets.value("strategy/mode", true).toBool();
        lSensorIDs.clear();
        int size = sets.beginReadArray("strategy/sensorids");
        for(int i = 0; i < size; i++)
        {
            sets.setArrayIndex(i);
            int id = sets.value("id", -1).toInt();
            lSensorIDs.push_back(id);
        }
        sets.endArray();
        //时间间隔等配置 system
        iTimeInterval_GeneRecord = sets.value("system/time_interval_generate_record", 0).toInt();
        iTimeInterval_GetSrcData = sets.value("system/time_interval_get_src_data", 60).toInt();
        iDaysDataOutDate = sets.value("system/data_outdate_days", 30).toInt();

        //系统语言及词条文件相关配置
        iLanguage = sets.value("system/language", 0).toInt();
		sCodetsprefix = sets.value("system/code_file_prefix", "Words").toString();
		bReadSrcData = sets.value("system/isreadsrcdata", false).toBool();
		bSaveSrcData = sets.value("system/issavesrcdata", false).toBool();
		iLogLevel = sets.value("system/loglevel", 2).toInt();

		size = sets.beginReadArray("class");
		for(int i = 0; i < size; i++)
		{
			sets.setArrayIndex(i);
			QString t = sets.value("time", "00:00:00").toString();
			lShiftTime.push_back(QTime::fromString("hh:mm:ss"));
		}
		if(lShiftTime.size() == 0)
		{
			lShiftTime.append(QTime::fromString("00:00:00"));
			sets.beginWriteArray("Shift", lShiftTime.size());
			for(int i = 0; i < lShiftTime.size(); i++)
			{
				sets.setArrayIndex(i);
				sets.setValue("time", lShiftTime[i].toString("hh:mm:ss"));
			}
			sets.endArray();
		}
    }

    void Save()
    {
        QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //策略相关配置
        sets.setValue("strategy/mode", bStrategyMode);
        sets.beginWriteArray("strategy/sensorids", lSensorIDs.size());
        for(int i = 0; i < lSensorIDs.size(); i++)
        {
            sets.setArrayIndex(i);
            sets.setValue("id", lSensorIDs[i]);
        }
        sets.endArray();

        //时间间隔等配置 system
        sets.setValue("system/time_interval_generate_record", iTimeInterval_GeneRecord);
        sets.setValue("system/time_interval_get_src_data", iTimeInterval_GetSrcData);
        sets.setValue("system/data_outdate_days", iDaysDataOutDate);

        //系统语言及词条文件相关配置
        sets.setValue("system/language", iLanguage);
		sets.setValue("system/code_file_prefix", sCodetsprefix);
		sets.setValue("system/isreadsrcdata", bReadSrcData);
		sets.setValue("system/issavesrcdata", bSaveSrcData);
		sets.setValue("system/loglevel", iLogLevel);
		//班次
		sets.beginWriteArray("Shift", lShiftTime.size());
		for(int i = 0; i < lShiftTime.size(); i++)
		{
			sets.setArrayIndex(i);
			sets.setValue("time", lShiftTime[i].toString("hh:mm:ss"));
		}
		sets.endArray();
        sets.sync();
    }
	void SaveDefaultValue()
	{
		QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //策略相关配置
        sets.setValue("strategy/mode", true);
		sets.beginWriteArray("strategy/sensorids", 2);
		sets.setArrayIndex(0);
		sets.setValue("id", 10);
		sets.setArrayIndex(1);
		sets.setValue("id", 34);
        sets.endArray();

        //时间间隔等配置 system
        sets.setValue("system/time_interval_generate_record", 5);
        sets.setValue("system/time_interval_get_src_data", 5);
        sets.setValue("system/data_outdate_days", 30);

        //系统语言及词条文件相关配置
        sets.setValue("system/language", 0);
		sets.setValue("system/code_file_prefix", "CodeTs");
		sets.setValue("system/isreadsrcdata", false);
		sets.setValue("system/issavesrcdata", false);
		sets.setValue("system/loglevel", 2);
        sets.sync();
	}

public:
    bool bStrategyMode;                 //策略模式 true表示 白名单 false表示黑名单
    QList<int> lSensorIDs;               //策略影响的缺陷ID列表
    int iTimeInterval_GeneRecord;       //时间间隔,用于在多长时间生成一条统计记录 1:60分钟 2:90分钟;3:120分钟 其他:30分钟 具体使用由DataCenter决定
    int iDaysDataOutDate;               //原始数据有效天数 单位:天,超过此时间,数据库会删除
	bool bReadSrcData;					//是否读取源数据文件
	bool bSaveSrcData;					//是否保存源数据到数据文件
	int iLogLevel;						//日志级别
	int iDefaultShow;					//界面默认显示 0:按时间间隔显示 1:按班次显示

    int iTimeInterval_GetSrcData;       //获取原始数据的时间间隔 单位:秒

    int iLanguage;                       //语言 0:中文 1:英文 涉及到界面显示语言和使用的词条对应文件名
    QString sCodetsprefix;              //词条文件前缀 如配置为Error,语言配置为0,则实际使用文件为Error_zh.ini
	/*班次时间定义说明:
	1 班次的所属日期为整天时间占比最多的那个日期 
	2 第一个设定时间开始为一天的统计起始时间
	3 如果前一个时间比后一个时间大(后一个时间比前一个时间小),则后一个时间在前一个时间的下一个日期的时间
	4 一天班次有几个,则设定的时间点则有几个,最后一个时间点到下一个第一个时间点之间的时间段为班次的最后一个班
	5 班次定义为24小时,所以所有班次时间点定义 跨时间只会存在一次,
	*/
	QList<QTime> lShiftTime;			//班次时间点的配置 
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

void Config::GetCoreConf(CoreConf& cfg)
{
	cfg.bReadSrcData = pimpl->bReadSrcData;
	cfg.bSaveSrcData = pimpl->bSaveSrcData;
	cfg.iTimeInterval_GetSrcData = pimpl->iTimeInterval_GetSrcData;
}

int Config::GetConfigLogLevel()
{
	return pimpl->iLogLevel;
}

