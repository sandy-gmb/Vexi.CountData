#include "dataview.h"
#include "mainui.h"
#include "setting.h"
#include "querydata.h"

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
    SettingWidget* m_settingui; 
    QueryWidget* m_query;
};

DataView::DataView( QObject* parent)
    : QObject(parent)
    , pimpl(new Impl)
{
    pimpl->m_mainui = new MainUI(this);
    pimpl->m_query = new QueryWidget(this);
    pimpl->m_settingui = new SettingWidget(this);

    connect(pimpl->m_mainui, SIGNAL(closed( )), this, SIGNAL(closed( )));
}

DataView::~DataView()
{
    pimpl->m_settingui->close();
    pimpl->m_query->close();
    pimpl->m_mainui->close();
    delete pimpl;
}

void DataView::show()
{
    OnChangeUI(EUI_Main);
}

void DataView::OnLanguageChange(int lang)
{
    //设置使用的翻译文件
    QString tsfile = "DataView";
    if((ELanguage)lang == EL_English){
        tsfile += "_en.qm";
    }else
    {
        tsfile += "_zh.qm";
    }
    static QTranslator* pTran;
    if (pTran != NULL)
    {
        qApp->removeTranslator(pTran);
        delete pTran;
        pTran = NULL;
    }
    pTran = new QTranslator;
    pTran->load(tsfile);
    qApp->installTranslator(pTran);

    QMap<QString, QString> moldwors;
    QMap<QString, QString> sensorwors;
    signal_GetWordsTranslation(moldwors, sensorwors);
    pimpl->m_mainui->ChangeLanguage(moldwors, sensorwors);
    pimpl->m_query->ChangeLanguage(moldwors, sensorwors);
}


void DataView::OnChangeUI(EUISelection sel)
{
    switch(sel)
    {
    case EUI_Main:
        pimpl->m_mainui->refreshWidget();
        pimpl->m_mainui->show();
        pimpl->m_query->hide();
        pimpl->m_settingui->hide();
        break;
    case EUI_QueryData:
        pimpl->m_query->on_btn_refresh_clicked();
        pimpl->m_query->show();
        pimpl->m_mainui->hide();
        pimpl->m_settingui->hide();
        break;
    case EUI_Settings:
        pimpl->m_settingui->on_btn_reload_clicked();
        pimpl->m_settingui->show();
        pimpl->m_query->hide();
        pimpl->m_mainui->hide();
        break;
    }
}

void DataView::OnRecordShowChanged(int show)
{//更新信息
    pimpl->m_mainui->ChangedShowRecordT((ERecordType)show);
}

void DataView::Init(QString title)
{
    AllConfig cfg;
    signal_GetAllConfig(cfg);
    OnLanguageChange(cfg.syscfg.iLanguage);
    if(title != "")
    {
        pimpl->m_mainui->Init((ERecordType)cfg.syscfg.iDefaultShow, title);
    }
    else
    {
        pimpl->m_mainui->Init((ERecordType)cfg.syscfg.iDefaultShow);
    }
    pimpl->m_query->Init();
    pimpl->m_settingui->Init();
    if(title != "")
    {
        pimpl->m_mainui->setWindowTitle(title);
    }
}
