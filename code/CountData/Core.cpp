#include "Core.hpp"
#include "soap/soapH.h"
#include "soap/ServiceTM11Soap.nsmap"
#include "ConfigDef.hpp"

#include "logger.h"
#include <sstream>

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
    signal_GetCoreConf(cfg);
	
	// WebService调用对象
	class _ns1__Counts getMobileCodeObject;
	// WebService返回对象
	class _ns1__CountsResponse getMobileCodeResponseObject;

	fstream ifs;
	QStringList srcdata;
	if(cfg.dbgcfg.bReadSrcData)
	{
		ifs.open("srcdata.txt", ios::in );
		if(ifs.is_open())
		{
			string t;
			std::getline(ifs, t);
			while(t != "")
			{
				srcdata.append(QString::fromStdString(t));
				std::getline(ifs, t);
			}
			ifs.close();
		}
	}
	if(srcdata.isEmpty())
	{
		// SOAP初始化
		soap_init(&pimpl->m_oSoap);
		// 调用函数的参数赋值
		ELOGI("Soap Init Complete");
	}
    
    int nResult = -1;
	int idx = 0;
    while (isrun)
    {//每半个小时执行一次
        int lastres = nResult;
		if(srcdata.isEmpty() || !cfg.dbgcfg.bReadSrcData)
		{
			// 发送WebService请求，并获得返回结果
			nResult = soap_call___ns1__Counts(&pimpl->m_oSoap,NULL,NULL,&getMobileCodeObject,getMobileCodeResponseObject);
		}
		else
		{
			nResult = SOAP_OK;
		}

        // 操作成功
        if(SOAP_OK == nResult)
        {
			QString xmlstr;
			if(cfg.dbgcfg.bReadSrcData && !srcdata.isEmpty())
			{
				xmlstr = srcdata[idx];
				idx = (idx+1)%srcdata.size();
			}
			else
			{ // 输出返回结果
				char* strResult = getMobileCodeResponseObject.CountsResult->__any;
				xmlstr = QString::fromLocal8Bit(strResult);
				ELOGD("receive data:%s ", qPrintable(xmlstr));
			}
           
            QString err;
			if(!cfg.dbgcfg.bReadSrcData && cfg.dbgcfg.bSaveSrcData)
			{
				ifs.open("srcdata.txt", ios::out | ios::app);
				if( ifs.is_open())
				{
					ifs<<xmlstr.toStdString()<<endl;
					ifs.close();
				}
			}
			
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

        Sleep(cfg.syscfg.iTimeInterval_GetSrcData *1000);
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

