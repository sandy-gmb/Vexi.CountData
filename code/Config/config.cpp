#include "config.hpp"
#include <QSettings>
#include <QTextCodec>
#include <QVariant>

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
        QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //�����������
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
        //ʱ���������� system
        iTimeInterval_GeneRecord = sets.value("system/time_interval_generate_record", 0).toInt();
        iTimeInterval_GetSrcData = sets.value("system/time_interval_get_src_data", 60).toInt();
        iDaysDataOutDate = sets.value("system/data_outdate_days", 30).toInt();

        //ϵͳ���Լ������ļ��������
        iLanguage = sets.value("system/language", 0).toInt();
        sCodetsprefix = sets.value("system/code_file_prefix", "Words").toString();
        
    }

    void Save()
    {
        QSettings sets(QString("%1/%2").arg(ConfigDir).arg(ConfigFile),QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        //�����������
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

        //ʱ���������� system
        sets.setValue("system/time_interval_generate_record", iTimeInterval_GeneRecord);
        sets.setValue("system/time_interval_get_src_data", iTimeInterval_GetSrcData);
        sets.setValue("system/data_outdate_days", iDaysDataOutDate);

        //ϵͳ���Լ������ļ��������
        sets.setValue("system/language", iLanguage);
        sets.setValue("system/code_file_prefix", sCodetsprefix);
        sets.sync();
    }

public:
    bool bStrategyMode;                 //����ģʽ true��ʾ ������ false��ʾ������
    QList<int> lSensorIDs;               //����Ӱ���ȱ��ID�б�
    int iTimeInterval_GeneRecord;       //ʱ����,�����ڶ೤ʱ������һ��ͳ�Ƽ�¼ 1:60���� 2:90����;3:120���� ����:30���� ����ʹ����DataCenter����
    int iDaysDataOutDate;               //ԭʼ������Ч���� ��λ:��,������ʱ��,���ݿ��ɾ��
    bool bIsLogEveryData;               //�Ƿ���־��¼��������

    int iTimeInterval_GetSrcData;       //��ȡԭʼ���ݵ�ʱ���� ��λ:��

    int iLanguage;                       //���� 0:���� 1:Ӣ�� �漰��������ʾ���Ժ�ʹ�õĴ�����Ӧ�ļ���
    QString sCodetsprefix;              //�����ļ�ǰ׺ ������ΪError,��������Ϊ0,��ʵ��ʹ���ļ�ΪError_zh.ini
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


