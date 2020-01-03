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
    // SOAP初始化
    soap_init(&pimpl->m_oSoap);
}

Core::~Core()
{
    isrun = false;
    
    /// 关闭SOAP
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
    // WebService调用对象
    class _ns1__Counts getMobileCodeObject;
    // WebService返回对象
    class _ns1__CountsResponse getMobileCodeResponseObject;
    // SOAP初始化
    soap_init(&pimpl->m_oSoap);
    // 调用函数的参数赋值
    ELOGI("Soap Init Complete");
    int nResult = -1;
    while (isrun)
    {//每半个小时执行一次
        int lastres = nResult;
        // 发送WebService请求，并获得返回结果
        nResult = soap_call___ns1__Counts(&pimpl->m_oSoap,NULL,NULL,&getMobileCodeObject,getMobileCodeResponseObject);
        // 操作成功
        if(SOAP_OK == nResult)
        {
            // 输出返回结果
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

