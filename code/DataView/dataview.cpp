#include "dataview.h"
#include "mainui.h"
#include "setting.h"

#include <QTranslator>
#include <QCoreApplication>
#include <QMap>
#include <QStringList>
#include <QSettings>
#include <QTextCodec>


class DataView::Impl
{
public:
    MainUI* m_mainui;
    setting* m_settingui; 

    QMap<int, QString> m_moldwors;        //模板ID词条
    QMap<int, QString> m_sensorwors;      //缺陷ID词条
};

DataView::DataView(int lang, QObject* parent)
    : QObject(parent)
    , pimpl(new Impl)
{
    //设置使用的翻译文件
    QString tsfile = "DataView";
    if(lang == 1){
        tsfile += "_en.qm";
    }else
    {
        tsfile += "_zh.qm";
    }

    QTranslator *pTran = new QTranslator(qApp);
    pTran->load(tsfile);
    qApp->installTranslator(pTran);

    pimpl->m_mainui = new MainUI;
    pimpl->m_settingui = new setting(pimpl->m_mainui);
    pimpl->m_mainui->m_settingui = pimpl->m_settingui;

    connect(pimpl->m_mainui, SIGNAL(signal_GetLastestRecord(int, Record& , QString* )), this, SIGNAL(signal_GetLastestRecord(int, Record& , QString* )));
    connect(pimpl->m_mainui, SIGNAL(signal_GetTimeInterval( )), this, SIGNAL(signal_GetTimeInterval( )));
    connect(pimpl->m_mainui, SIGNAL(closed( )), this, SIGNAL(closed( )));
    connect(pimpl->m_settingui, SIGNAL(signal_GetAllDate(int, QList<QDate>& )), this, SIGNAL(signal_GetAllDate(int, QList<QDate>& )));
    connect(pimpl->m_settingui, SIGNAL(signal_GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>& )), this, SIGNAL(signal_GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>& )));
    connect(pimpl->m_settingui, SIGNAL(signal_GetRecordByTime(QDateTime , QDateTime , Record& )), this, SIGNAL(signal_GetRecordByTime(QDateTime , QDateTime , Record& )));
    connect(pimpl->m_settingui, SIGNAL(signal_GetTimeInterval()), this, SIGNAL(signal_GetTimeInterval()));
    connect(pimpl->m_settingui, SIGNAL(signal_SetTimeInterval(ETimeInterval )), this, SIGNAL(signal_SetTimeInterval(ETimeInterval )));

}

DataView::~DataView()
{
    pimpl->m_settingui->close();
    pimpl->m_mainui->close();
    delete pimpl;
}

void DataView::show()
{
    pimpl->m_settingui->hide();
    pimpl->m_mainui->show();
}

void DataView::Init()
{
    {//解析词条文件 
        QString filep = GetWordsTranslationFilePath();
        QSettings sets(filep, QSettings::IniFormat);
        sets.setIniCodec(QTextCodec::codecForName("UTF-8"));
        
        sets.beginGroup("MoldWords");
        QStringList keys = sets.childKeys();
        foreach(QString key, keys)
        {
            pimpl->m_moldwors.insert(key.toInt(), sets.value(key).toString()  ) ;
        }
        sets.endGroup();
        sets.beginGroup("SensorWords");
        keys = sets.childKeys();
        foreach(QString key, keys)
        {
            pimpl->m_sensorwors.insert(key.toInt(), sets.value(key).toString() ) ;
        }
        sets.endGroup();
    }
    pimpl->m_mainui->m_moldwors = pimpl->m_moldwors;
    pimpl->m_mainui->m_sensorwors = pimpl->m_sensorwors;
    pimpl->m_settingui->m_moldwors = pimpl->m_moldwors;
    pimpl->m_settingui->m_sensorwors = pimpl->m_sensorwors;

    pimpl->m_mainui->Init();
    pimpl->m_settingui->Init();

}
