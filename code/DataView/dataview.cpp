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

    QMap<int, QString> m_moldwors;        //模板ID词条
    QMap<int, QString> m_sensorwors;      //缺陷ID词条
};

DataView::DataView( QObject* parent)
    : QObject(parent)
    , pimpl(new Impl)
{
    pimpl->m_mainui = new MainUI(this);
	pimpl->m_query = new QueryWidget(this);
    pimpl->m_settingui = new SettingWidget(this);

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

void DataView::OnLanguageChange(ELanguage lang)
{
	//设置使用的翻译文件
	QString tsfile = "DataView";
	if(lang == EL_English){
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

	QMap<int, QString> moldwors;
	QMap<int, QString> sensorwors;
	signals_GetWordsTranslation(moldwors, sensorwors);
	pimpl->m_mainui->ChangeLanguage(moldwors, sensorwors);
	pimpl->m_query->ChangeLanguage(moldwors, sensorwors);
	pimpl->m_settingui->ChangeLanguage();
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
		AllConfig cfg;
		signals_GetAllConfig(cfg);

		//pimpl->m_settingui->refreshWidget();
		pimpl->m_settingui->show();
		pimpl->m_query->hide();
		pimpl->m_mainui->hide();
		break;
	}
}

void DataView::OnRecordConfigChanged()
{//更新信息

}

void DataView::Init()
{
	pimpl->m_mainui->Init();
	pimpl->m_query->Init();
    pimpl->m_settingui->Init();
}
