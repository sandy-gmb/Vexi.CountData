#include "datacenter.h"

// datacenter.cpp : 定义 DLL 应用程序的导出函数。
//

#include "PaserData.h"
#include "SaveDataToDB.h"

#include <logger.h>

using namespace std;

class DataCenter::Impl
{
public:
    Impl()
    {
        lastt = QDateTime::currentDateTime();
    }

    QDateTime lastt;        //上一次的时间
    SaveDataToDB m_db;
};



DataCenter::DataCenter()
    : m_pimpl(new Impl)
{
}

DataCenter::~DataCenter()
{
    delete m_pimpl;
    m_pimpl = nullptr;
}

bool DataCenter::Init(QString* err)
{
    return m_pimpl->m_db.Init(err);
}

bool DataCenter::PaserDataToDataBase(const QString& xmldata, QString* err)
{
    if(xmldata == "")
    {
        m_pimpl->lastt = QDateTime::currentDateTime();
        SAFE_SET(err, QObject::tr("No Input Data"));
        return false;
    }

    if(m_pimpl->m_db.m_islogevery_data)
    {
        ELOGI(xmldata.toLocal8Bit().constData());
    }

    XmlData data;
    data.dt_start = m_pimpl->lastt;
    if(!XMLPaser::PaserInfo(xmldata, data, err)) 
    {//解析数据出错
        ELOGE("%s  %s", xmldata.toLocal8Bit().constData(), err->toLocal8Bit().constData());
        return false;
    }
    m_pimpl->lastt = QDateTime::currentDateTime();
    //将数据保存到数据库
    bool res = m_pimpl->m_db.SaveData(data, err);
    if(!res)
    {
        ELOGE(err->toLocal8Bit().constData());
    }
    return res;
}

bool DataCenter::GetLastestRecord( Record& data, QString* err )
{
    return m_pimpl->m_db.GetLastestRecord(data, err);
}

ETimeInterval DataCenter::GetTimeInterval()
{
    return m_pimpl->m_db.GetTimeInterval();
}

void DataCenter::SetTimeInterval( ETimeInterval timeinterval )
{
    return m_pimpl->m_db.SetTimeInterval(timeinterval);
}

bool DataCenter::GetAllDate( QList<QDate>& lst )
{
    return m_pimpl->m_db.GetAllDate(lst);
}

bool DataCenter::GetRecordListByDay( QDate date, QList<QTime>& stlst, QList<QTime>& etlst )
{
    return m_pimpl->m_db.GetRecordListByDay(date, stlst, etlst);
}

bool DataCenter::GetRecordByTime( QDateTime st, QDateTime end, Record& data )
{
    return m_pimpl->m_db.GetRecordByTime(st, end, data);
}

void DataCenter::Stop()
{
    m_pimpl->m_db.Stop();
}
