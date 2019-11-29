#include "dataview.h"
#include "mainui.h"
#include "setting.h"


class DataView::Impl
{
public:
    MainUI* m_mainui;
    setting* m_settingui; 
};

DataView::DataView(QObject* parent)
    : QObject(parent)
    , pimpl(new Impl)
{
    pimpl->m_mainui = new MainUI;
    pimpl->m_settingui = new setting(pimpl->m_mainui);
    pimpl->m_mainui->m_settingui = pimpl->m_settingui;

    connect(pimpl->m_mainui, SIGNAL(signal_GetLastestRecord(Record& , QString* )), this, SIGNAL(signal_GetLastestRecord(Record& , QString* )));
    connect(pimpl->m_mainui, SIGNAL(signal_GetTimeInterval( )), this, SIGNAL(signal_GetTimeInterval( )));
    connect(pimpl->m_mainui, SIGNAL(closed( )), this, SIGNAL(closed( )));
    connect(pimpl->m_settingui, SIGNAL(signal_GetAllDate(QList<QDate>& )), this, SIGNAL(signal_GetAllDate(QList<QDate>& )));
    connect(pimpl->m_settingui, SIGNAL(signal_GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>& )), this, SIGNAL(signal_GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>& )));
    connect(pimpl->m_settingui, SIGNAL(signal_GetRecordByTime(QDateTime , QDateTime , Record& )), this, SIGNAL(signal_GetRecordByTime(QDateTime , QDateTime , Record& )));
    connect(pimpl->m_settingui, SIGNAL(signal_GetTimeInterval()), this, SIGNAL(signal_GetTimeInterval()));
    connect(pimpl->m_settingui, SIGNAL(signal_SetTimeInterval(ETimeInterval )), this, SIGNAL(signal_SetTimeInterval(ETimeInterval )));

    pimpl->m_mainui->Init();
    pimpl->m_settingui->Init();
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
