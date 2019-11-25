// PaserDataToDataBase.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "PaserDataToDataBase.h"
#include "PaserData.h"
#include "SaveDataToDB.h"

#include <fstream>
#include <iostream>

using namespace std;

class CPaserDataToDataBase::Impl
{
public:
    Impl()
    {
        lastt = QDateTime::currentDateTime();
    }

    QDateTime lastt;        //上一次的时间
    SaveDataToDB m_db;
};



CPaserDataToDataBase::CPaserDataToDataBase()
    : m_pimpl(new Impl)
{
}

CPaserDataToDataBase::~CPaserDataToDataBase()
{
    delete m_pimpl;
    m_pimpl = nullptr;
}

bool CPaserDataToDataBase::Init(QString* err)
{
    return m_pimpl->m_db.Init(err);
}

bool CPaserDataToDataBase::PaserDataToDataBase(const QString& xmldata, QString* err)
{
    if(xmldata == "")
    {
        m_pimpl->lastt = QDateTime::currentDateTime();
        SAFE_SET(err, QObject::tr("No Input Data"));
        return false;
    }
   
    if(m_pimpl->m_db.m_islogevery_data)
    {
        fstream fs;
        fs.open("./log.txt", ios::app);
        if(fs.is_open())
        {
            fs<<xmldata.toLocal8Bit().constData()<<endl;
            fs.close();
        }
    }

    XmlData data;
    data.dt_start = m_pimpl->lastt;
    if(!XMLPaser::PaserInfo(xmldata, data, err)) 
    {//解析数据出错
        fstream fs;
        fs.open("./log.txt", ios::app);
        if(fs.is_open())
        {
            fs<<xmldata.toLocal8Bit().constData()<<"  "<<err->toLocal8Bit().constData()<<endl;
            fs.close();
        }

        return false;
    }
    m_pimpl->lastt = QDateTime::currentDateTime();
    //将数据保存到数据库
    bool res = m_pimpl->m_db.SaveData(data, err);
    if(!res)
    {
        fstream fs;
        fs.open("./log.txt", ios::app);
        if(fs.is_open())
        {
            fs<<err->toLocal8Bit().constData()<<endl;
            fs.close();
        }
    }
    return res;
}