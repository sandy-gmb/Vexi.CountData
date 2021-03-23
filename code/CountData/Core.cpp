#include "Core.hpp"
#include "soap/soapH.h"
#include "soap/ServiceTM11Soap.nsmap"

#include "protocolv2/protocolv2H.h"

#include "ConfigDef.hpp"

#include "logger.h"
#include <sstream>

#include <QCoreApplication>

#define DefaultTimeInterval 60000 //1000*60

#include <fstream>
#include <QStringList>
using namespace std;

class Core::Impl
{
public:
    struct soap m_oSoap;
};

Core::Core(QObject* parent)
    : QThread(parent)
    , pimpl(new Impl)
{
    timeinterval = DefaultTimeInterval;
    isrun = true;
    // SOAP��ʼ��
    soap_init(&pimpl->m_oSoap);
}

Core::~Core()
{
    isrun = false;
    QTime time;
    time.start();
    while (time.elapsed() < cfg.syscfg.iTimeInterval_GetSrcData * 1000 && !isexit) {
        QCoreApplication::processEvents();
    }
    /// �ر�SOAP
    soap_destroy(&pimpl->m_oSoap);
    soap_end(&pimpl->m_oSoap);
    soap_done(&pimpl->m_oSoap);
    if(isexit)
    {
        delete pimpl;
    }
}

void Core::run()
{
    isexit = false;
    signal_GetCoreConf(cfg);

    fstream ifs;
    QStringList srcdata;
    if (cfg.dbgcfg.bReadSrcData)
    {
        ifs.open("srcdata.txt", ios::in);
        if (ifs.is_open())
        {
            string t;
            std::getline(ifs, t);
            while (t != "")
            {
                srcdata.append(QString::fromStdString(t));
                std::getline(ifs, t);
            }
            ifs.close();
        }
    }
    if (srcdata.isEmpty())
    {
        // SOAP��ʼ��
        soap_init(&pimpl->m_oSoap);
        // ���ú����Ĳ�����ֵ
        ELOGI("Soap Init Complete");
    }

    switch (cfg.syscfg.iProtocolVersion)
    {
    case EPV_VEXI:
        runVexiProtocolV1(srcdata);
        break;
    case EPV_TERA:
        runProtocolV2(srcdata);
        break;
    default:
        break;
    }
    
    ELOGI("Core Exit");
}

void Core::stop()
{
    isrun = false;
    wait(1000);
}

void Core::OnCoreConfChange(const CoreConf& _cfg)
{
    cfg = _cfg;
}

void Core::runVexiProtocolV1(const QStringList& srcdata)
{
    // WebService���ö���
    class _ns1__Counts getMobileCodeObject;
    // WebService���ض���
    class _ns1__CountsResponse getMobileCodeResponseObject;

    int nResult = -1;
    int idx = 0;
    while (isrun)
    {//ÿ���Сʱִ��һ��
        int lastres = nResult;
        if (srcdata.isEmpty() || !cfg.dbgcfg.bReadSrcData)
        {
            // ����WebService���󣬲���÷��ؽ��
            //http://127.0.0.1/wstm11/service.asmx/Counts

            nResult = soap_call___ns1__Counts(&pimpl->m_oSoap, NULL, NULL, &getMobileCodeObject, getMobileCodeResponseObject);
        }
        else
        {
            nResult = SOAP_OK;
        }

        // �����ɹ�
        if (SOAP_OK == nResult)
        {
            QString xmlstr;
            if (cfg.dbgcfg.bReadSrcData && !srcdata.isEmpty())
            {
                xmlstr = srcdata[idx];
                idx = (idx + 1) % srcdata.size();
            }
            else
            { // ������ؽ��
                char* strResult = getMobileCodeResponseObject.CountsResult->__any;
                xmlstr = QString::fromLocal8Bit(strResult);
                ELOGD("receive data:%s ", qPrintable(xmlstr));
            }

            QString err;
            if (!cfg.dbgcfg.bReadSrcData && cfg.dbgcfg.bSaveSrcData)
            {
                fstream ifs;
                ifs.open("srcdata.txt", ios::out | ios::app);
                if (ifs.is_open())
                {
                    ifs << xmlstr.toStdString() << endl;
                    ifs.close();
                }
            }

            if (!signal_PaserDataToDataBase(xmlstr, &err))
            {
                ELOGD("Parser Count Data To DataBase Error:%s ", qPrintable(err));
            }
        }
        else
        {
            if (lastres != nResult)
            {
                std::ostringstream oss;
                soap_stream_fault(&pimpl->m_oSoap, oss);
                ELOGE("Webservice communication Error:%d %s", nResult, oss.str().c_str());
            }
        }
        if (!isrun)
            return;
        Sleep(cfg.syscfg.iTimeInterval_GetSrcData * 1000);
    }
}

void Core::runProtocolV2(const QStringList& srcdata)
{
    // WebService���ö���
    class protocolv2::_ns1__Counts getMobileCodeObject;
    // WebService���ض���
    class  protocolv2::_ns1__CountsResponse getMobileCodeResponseObject;

    int nResult = -1;
    int idx = 0;
    while (isrun)
    {//ÿ���Сʱִ��һ��
        int lastres = nResult;
        if (srcdata.isEmpty() || !cfg.dbgcfg.bReadSrcData)
        {
            // ����WebService���󣬲���÷��ؽ��
            //http://127.0.0.1/webservice/cwebservice.asmx

            nResult = protocolv2::soap_call___ns1__Counts(&pimpl->m_oSoap, NULL, NULL, &getMobileCodeObject, getMobileCodeResponseObject);
        }
        else
        {
            nResult = SOAP_OK;
        }

        // �����ɹ�
        if (SOAP_OK == nResult)
        {
            QString xmlstr;
            if (cfg.dbgcfg.bReadSrcData && !srcdata.isEmpty())
            {
                xmlstr = srcdata[idx];
                idx = (idx + 1) % srcdata.size();
            }
            else
            { // ������ؽ��
                char* strResult = getMobileCodeResponseObject.CountsResult;
                xmlstr = QString::fromLocal8Bit(strResult);
                ELOGD("receive data:%s ", qPrintable(xmlstr));
            }

            QString err;
            if (!cfg.dbgcfg.bReadSrcData && cfg.dbgcfg.bSaveSrcData)
            {
                fstream ifs;
                ifs.open("srcdata.txt", ios::out | ios::app);
                if (ifs.is_open())
                {
                    ifs << xmlstr.toStdString() << endl;
                    ifs.close();
                }
            }

            if (!signal_PaserDataToDataBase(xmlstr, &err))
            {
                ELOGD("Parser Count Data To DataBase Error:%s ", qPrintable(err));
            }
        }
        else
        {
            if (lastres != nResult)
            {
                std::ostringstream oss;
                soap_stream_fault(&pimpl->m_oSoap, oss);
                ELOGE("Webservice communication Error:%d %s", nResult, oss.str().c_str());
            }
        }

        if (!isrun)
            return;
        Sleep(cfg.syscfg.iTimeInterval_GetSrcData * 1000);
    }
}

