#include "datacenter.h"

// datacenter.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "PaserData.h"
#include "SaveDataToDB.h"

#include <fstream>
#include <iostream>

using namespace std;

class DataCenter::Impl
{
public:
    Impl()
    {
        lastt = QDateTime::currentDateTime();
    }

    QDateTime lastt;        //��һ�ε�ʱ��
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
    {//�������ݳ���
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
    //�����ݱ��浽���ݿ�
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