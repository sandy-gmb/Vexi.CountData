#include "Core.hpp"
#include "soap/soapH.h"
#include "soap/ServiceTM11Soap.nsmap"
#include <QDebug>

#include "logger.h"
#include <sstream>

#define DefaultTimeInterval 60000 //1000*60

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
    
    /// �ر�SOAP
    soap_destroy(&pimpl->m_oSoap);
    soap_end(&pimpl->m_oSoap);
    soap_done(&pimpl->m_oSoap);

    delete pimpl;
}

void Core::run()
{
    int t = 0;
    t = GetTimeOfObtainSrcData();
    if( t > 0)
    {
        timeinterval = t;
    }
    // WebService���ö���
    class _ns1__Counts getMobileCodeObject;
    // WebService���ض���
    class _ns1__CountsResponse getMobileCodeResponseObject;
    // SOAP��ʼ��
    soap_init(&pimpl->m_oSoap);
    // ���ú����Ĳ�����ֵ
    ELOGI("Soap Init Complete");
    int nResult = -1;
    while (isrun)
    {//ÿ���Сʱִ��һ��
        int lastres = nResult;
        // ����WebService���󣬲���÷��ؽ��
        nResult = soap_call___ns1__Counts(&pimpl->m_oSoap,NULL,NULL,&getMobileCodeObject,getMobileCodeResponseObject);
        // �����ɹ�
        if(SOAP_OK == nResult)
        {
            // ������ؽ��
            char* strResult = getMobileCodeResponseObject.CountsResult->__any;
            QString xmlstr = QString::fromLocal8Bit(strResult);
            QString err;
            if(!signal_PaserDataToDataBase(xmlstr, &err))
            {
                ELOGD( "Parser Count Data To DataBase Error:%s ",qPrintable(err));
            }
        }
        else
        {
            if( lastres != nResult)
            {
                std::ostringstream oss;
                soap_stream_fault(&pimpl->m_oSoap, oss);
                ELOGE("Webservice communication Error:%d %s", nResult, oss.str().c_str());
            }
        }

        Sleep(timeinterval);
    }
    ELOGD("Core Exit");
}

void Core::stop()
{
    isrun = false;
}

