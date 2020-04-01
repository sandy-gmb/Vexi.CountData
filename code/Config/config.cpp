#include "config.hpp"
#include <QSettings>
#include <QTextCodec>
#include <QVariant>
#include <QFile>
#include <QTime>
#include <QSharedPointer>
#include <QStringList>

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
		//�����������
		cfgStrategy.bStrategyMode = sets.value("strategy/mode", false).toBool();
		int size = sets.beginReadArray("strategy/sensorids");
		for(int i = 0; i < size; i++)
		{
			sets.setArrayIndex(i);
			cfgStrategy.lSensorIDs.append(sets.value("id", -1).toInt());
		}
		sets.endArray();

		//ϵͳ����
		cfgSystem.iTimeInterval_GeneRecord = sets.value("system/time_interval_generate_record", 5).toInt();
		cfgSystem.iTimeInterval_GetSrcData = sets.value("system/time_interval_get_src_data", 5).toInt();
		cfgSystem.iDaysDataOutDate = sets.value("system/data_outdate_days", 30).toInt();
		cfgSystem.iDefaultShow = sets.value("system/show_record_type", 1).toInt();
		cfgSystem.iLanguage = sets.value("system/language", 0).toInt();
		cfgSystem.sCodetsprefix = sets.value("system/code_file_prefix", "CodeTs").toString();
		//ʱ����
		eTimeInterval = (ETimeInterval)sets.value("timeInterval/time_interval", 0).toInt();
		//���
		size = sets.beginReadArray("shift/shift");
		QList<QString> shifttime;
		for(int i = 0; i < size; i++)
		{
			sets.setArrayIndex(i);
			shifttime.append(sets.value("time", "00:00:00").toString()) ;
		}
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
		//��������
		cfgDebug.bReadSrcData = sets.value("debug/is_read_source_data", false).toBool();
		cfgDebug.bSaveSrcData = sets.value("debug/is_save_source_data", false).toBool();
		cfgDebug.iLogLevel = sets.value("debug/log_level", 2).toInt();
		sets.sync();
    }

    void Save()
    {
		QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
		sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
		//�����������
		sets.setValue("strategy/mode", cfgStrategy.bStrategyMode);
		sets.beginWriteArray("strategy/sensorids", cfgStrategy.lSensorIDs.size());
		for(int i = 0; i < cfgStrategy.lSensorIDs.size(); i++)
		{
			sets.setArrayIndex(i);
			sets.setValue("id", cfgStrategy.lSensorIDs[i]);
		}
		sets.endArray();

		//ϵͳ����
		sets.setValue("system/time_interval_generate_record", cfgSystem.iTimeInterval_GeneRecord);
		sets.setValue("system/time_interval_get_src_data", cfgSystem.iTimeInterval_GetSrcData);
		sets.setValue("system/data_outdate_days", cfgSystem.iDaysDataOutDate);
		sets.setValue("system/show_record_type", cfgSystem.iDefaultShow);
		sets.setValue("system/language", cfgSystem.iLanguage);
		sets.setValue("system/code_file_prefix", cfgSystem.sCodetsprefix);
		//ʱ����
		sets.setValue("timeInterval/time_interval", (int)eTimeInterval);
		//���
		sets.beginWriteArray("shift/shift", ptrShift->shifttime.size());
		for(int i = 0; i < ptrShift->shifttime.size(); i++)
		{
			sets.setArrayIndex(i);
			sets.setValue("time", ptrShift->shifttime[i].toString("hh:mm:ss"));
		}
		//��������
		sets.setValue("debug/is_read_source_data", cfgDebug.bReadSrcData);
		sets.setValue("debug/is_save_source_data", cfgDebug.bSaveSrcData);
		sets.setValue("debug/log_level", cfgDebug.iLogLevel);
		sets.sync();
    }

	void SaveDefaultValue()
	{
		QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //�����������
        sets.setValue("strategy/mode", true);
		sets.beginWriteArray("strategy/sensorids", 2);
		sets.setArrayIndex(0);
		sets.setValue("id", 10);
		sets.setArrayIndex(1);
		sets.setValue("id", 34);
        sets.endArray();

		//ϵͳ����
		sets.setValue("system/time_interval_generate_record", 5);
		sets.setValue("system/time_interval_get_src_data", 1);
		sets.setValue("system/data_outdate_days", 30);
		sets.setValue("system/language", 0);
		sets.setValue("system/show_record_type", 1);
		sets.setValue("system/code_file_prefix", "CodeTs");
		//ʱ����
		sets.setValue("timeInterval/time_interval", 5);
		//���
		sets.beginWriteArray("shift/shift", 1);
		sets.setArrayIndex(0);
		sets.setValue("time", "00:00:00");
		sets.endArray();
		//��������
		sets.setValue("debug/is_read_source_data", false);
		sets.setValue("debug/is_save_source_data", false);
		sets.setValue("debug/log_level", 2);
        sets.sync();
	}

	void ParseTranslationWords()
	{
		//���������ҵ���Ӧ���ļ���
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
		{//���������ļ� 
			QSettings sets(filep, QSettings::IniFormat);
			sets.setIniCodec(QTextCodec::codecForName("UTF-8"));

			sets.beginGroup("MoldWords");
			QStringList keys = sets.childKeys();
			foreach(QString key, keys)
			{
				moldwors.insert(key.toInt(), sets.value(key).toString()  ) ;
			}
			sets.endGroup();
			sets.beginGroup("SensorWords");
			keys = sets.childKeys();
			foreach(QString key, keys)
			{
				sensorwors.insert(key.toInt(), sets.value(key).toString() ) ;
			}
			sets.endGroup();
		}
	}

public:
	QSharedPointer<Shift> ptrShift;		//���������εĶ���

	StrategyConfig cfgStrategy;		//�����������
	SystemConfig cfgSystem;			//ϵͳ�������
	ETimeInterval eTimeInterval;	//ʱ���� ��ʱ������ʱ���ͳ�Ƽ�¼ 1:60���� 2:90����;3:120���� ����:30���� ����ʹ����DataCenter����
	DebugConfg cfgDebug;			//�����������

	//������ʾ�Ĵ���������� ��Ҫ�������Խ�����Ӧ�����ļ� ���ڽ�����ʾ
	QMap<int, QString> moldwors;	//ģ��ŷ���
	QMap<int, QString> sensorwors;	//ȱ��ID����
};	


//�ڲ�ʵ�ʹ���һ���ļ� ���������Ӧϵͳ ʹ�õ��ļ�Ϊһϵ�� 

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
	
	if(cfg.dbgcfg != pimpl->cfgDebug)
	{
		pimpl->cfgDebug = cfg.dbgcfg;
		debug = false;
	}
	if(cfg.syscfg != pimpl->cfgSystem)
	{
		if(cfg.syscfg.iDefaultShow == pimpl->cfgSystem.iDefaultShow)
		{
			show = false;
		}
		if(cfg.syscfg.iLanguage == pimpl->cfgSystem.iLanguage)
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

	if( !sys || !strategy || !timeinterval || shift)
	{
		DataCenterConf cfg;
		GetDataCenterConf(cfg);
		emit DataConfChange(cfg);
	}

	if(!show)
	{
		emit RecordShowChanged(cfg.syscfg.iDefaultShow);
	}

	if(!lang)
	{
		emit LanguageChanged(cfg.syscfg.iLanguage);
	}
	
}

int Config::GetConfigLogLevel()
{
	return pimpl->cfgDebug.iLogLevel;
}

void Config::GetWordsTranslation(const QMap<int, QString>& moldwors, const QMap<int, QString>& sensorwors)
{
	QString lang = "_zh";
	if(pimpl->cfgSystem.iLanguage == EL_English)
	{
		lang = "_en";
	}
	QString suffix = ".ini";
	QString prefix = pimpl->cfgSystem.sCodetsprefix;
	if(prefix == "")
	{
		pimpl->cfgSystem.sCodetsprefix = DefaultWordsFilePrefix;
	}
	//return QString("%1/%2%3%4").arg(ConfigDir).arg(prefix).arg(lang).arg(suffix);
}

int Config::GetTimeInterval()
{
	return (int)pimpl->eTimeInterval;
}

