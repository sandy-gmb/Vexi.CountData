#include "datacenter.h"

// datacenter.cpp : 定义 DLL 应用程序的导出函数。
//

#include "PaserData.h"
#include "DBOperation.h"

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
    DBOperation m_db;
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
    DataCenterConf conf;
    GetDataCenterConf(conf);

    m_pimpl->m_db.cfgStrategy = conf.strategy;

    m_pimpl->m_db.work->eti = conf.eTimeInterval;
	m_pimpl->m_db.work->cfgSystem = conf.syscfg;
    m_pimpl->m_db.work->shift = conf.ptrShift;

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

bool DataCenter::GetLastestRecord(int type,  Record& data, QString* err )
{
    return m_pimpl->m_db.GetLastestRecord(type, data, err);
}

ETimeInterval DataCenter::GetTimeInterval()
{
    return m_pimpl->m_db.GetTimeInterval();
}

void DataCenter::SetTimeInterval( ETimeInterval timeinterval )
{
    SetGenerateRecordTimeInterval((int)timeinterval);
    return m_pimpl->m_db.SetTimeInterval(timeinterval);
}

bool DataCenter::GetAllDate(int type,  QList<QDate>& lst )
{
    return m_pimpl->m_db.GetAllDate(type, lst);
}

bool DataCenter::GetRecordListByDay(int type,  QDate date, QList<QTime>& stlst, QList<QTime>& etlst )
{
    return m_pimpl->m_db.GetRecordTimeListByDay(type, date, stlst, etlst);
}

bool DataCenter::GetRecordByTime(int type,  QDateTime st, QDateTime end, Record& data )
{
    return m_pimpl->m_db.GetRecordByTime(type, st, end, data);
}

void DataCenter::RecordConfigChanged()
{
	m_pimpl->m_db.RecordConfigChanged();
}

void DataCenter::OnDataConfChange(const DataCenterConf& cfg)
{
	m_pimpl->m_db.OnDataConfChange(cfg);
}

void DataCenter::Stop()
{
    m_pimpl->m_db.Stop();
}
