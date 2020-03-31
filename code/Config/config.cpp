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
    EL_Chinese  = 0,         //����
    EL_English  = 1,         //Ӣ��
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
        //�����������
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
        //ʱ���������� system
        iTimeInterval_GeneRecord = sets.value("system/time_interval_generate_record", 0).toInt();
        iTimeInterval_GetSrcData = sets.value("system/time_interval_get_src_data", 60).toInt();
        iDaysDataOutDate = sets.value("system/data_outdate_days", 30).toInt();

        //ϵͳ���Լ������ļ��������
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
        //�����������
        sets.setValue("strategy/mode", bStrategyMode);
        sets.beginWriteArray("strategy/sensorids", lSensorIDs.size());
        for(int i = 0; i < lSensorIDs.size(); i++)
        {
            sets.setArrayIndex(i);
            sets.setValue("id", lSensorIDs[i]);
        }
        sets.endArray();

        //ʱ���������� system
        sets.setValue("system/time_interval_generate_record", iTimeInterval_GeneRecord);
        sets.setValue("system/time_interval_get_src_data", iTimeInterval_GetSrcData);
        sets.setValue("system/data_outdate_days", iDaysDataOutDate);

        //ϵͳ���Լ������ļ��������
        sets.setValue("system/language", iLanguage);
		sets.setValue("system/code_file_prefix", sCodetsprefix);
		sets.setValue("system/isreadsrcdata", bReadSrcData);
		sets.setValue("system/issavesrcdata", bSaveSrcData);
		sets.setValue("system/loglevel", iLogLevel);
		//���
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
        //�����������
        sets.setValue("strategy/mode", true);
		sets.beginWriteArray("strategy/sensorids", 2);
		sets.setArrayIndex(0);
		sets.setValue("id", 10);
		sets.setArrayIndex(1);
		sets.setValue("id", 34);
        sets.endArray();

        //ʱ���������� system
        sets.setValue("system/time_interval_generate_record", 5);
        sets.setValue("system/time_interval_get_src_data", 5);
        sets.setValue("system/data_outdate_days", 30);

        //ϵͳ���Լ������ļ��������
        sets.setValue("system/language", 0);
		sets.setValue("system/code_file_prefix", "CodeTs");
		sets.setValue("system/isreadsrcdata", false);
		sets.setValue("system/issavesrcdata", false);
		sets.setValue("system/loglevel", 2);
        sets.sync();
	}

public:
    bool bStrategyMode;                 //����ģʽ true��ʾ ������ false��ʾ������
    QList<int> lSensorIDs;               //����Ӱ���ȱ��ID�б�
    int iTimeInterval_GeneRecord;       //ʱ����,�����ڶ೤ʱ������һ��ͳ�Ƽ�¼ 1:60���� 2:90����;3:120���� ����:30���� ����ʹ����DataCenter����
    int iDaysDataOutDate;               //ԭʼ������Ч���� ��λ:��,������ʱ��,���ݿ��ɾ��
	bool bReadSrcData;					//�Ƿ��ȡԴ�����ļ�
	bool bSaveSrcData;					//�Ƿ񱣴�Դ���ݵ������ļ�
	int iLogLevel;						//��־����
	int iDefaultShow;					//����Ĭ����ʾ 0:��ʱ������ʾ 1:�������ʾ

    int iTimeInterval_GetSrcData;       //��ȡԭʼ���ݵ�ʱ���� ��λ:��

    int iLanguage;                       //���� 0:���� 1:Ӣ�� �漰��������ʾ���Ժ�ʹ�õĴ�����Ӧ�ļ���
    QString sCodetsprefix;              //�����ļ�ǰ׺ ������ΪError,��������Ϊ0,��ʵ��ʹ���ļ�ΪError_zh.ini
	/*���ʱ�䶨��˵��:
	1 ��ε���������Ϊ����ʱ��ռ�������Ǹ����� 
	2 ��һ���趨ʱ�俪ʼΪһ���ͳ����ʼʱ��
	3 ���ǰһ��ʱ��Ⱥ�һ��ʱ���(��һ��ʱ���ǰһ��ʱ��С),���һ��ʱ����ǰһ��ʱ�����һ�����ڵ�ʱ��
	4 һ�����м���,���趨��ʱ������м���,���һ��ʱ��㵽��һ����һ��ʱ���֮���ʱ���Ϊ��ε����һ����
	5 ��ζ���Ϊ24Сʱ,�������а��ʱ��㶨�� ��ʱ��ֻ�����һ��,
	*/
	QList<QTime> lShiftTime;			//���ʱ�������� 
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

