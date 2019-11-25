#include <QApplication>
#include <iostream>
#include "soap/soapH.h"
#include "soap/ServiceTM11Soap.nsmap"
#include <QFile>
#include <QDebug>
#include <QStringList>
#include <fstream>
#include <atlbase.h>
#include <atlconv.h>

#include "datacenter.h"
#include <iostream>

using namespace std;

void testParserXmlData()
{
    DataCenter parser;
    QString err;
    if(!parser.Init(&err))
    {
        cout<<err.toLocal8Bit().constData()<<endl;
        return;
    }

    QString xmlstr = "<Root xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"\"> <Machine Id=\"MX\"> <Inspected>1</Inspected> <Rejects>1</Rejects> <Defects>1</Defects> <Autoreject>0</Autoreject> <Mold id=\"0\"> <Inspected>1</Inspected> <Rejects>1</Rejects> <Defects>1</Defects> <Autoreject>0</Autoreject> <Sensor id=\"40\"> <Rejects>0</Rejects> <Defects>0</Defects> </Sensor> <Sensor id=\"41\"> <Rejects>1</Rejects> <Defects>1</Defects> <Counter id=\"1\" Nb=\"1\"/> </Sensor> <Sensor id=\"42\"> <Rejects>0</Rejects> <Defects>0</Defects> </Sensor> <Sensor id=\"46\"> <Rejects>1</Rejects> <Defects>1</Defects> <Counter id=\"1\" Nb=\"1\"/> <Counter id=\"2\" Nb=\"1\"/> </Sensor> </Mold> <Mold id=\"2\"> <Inspected>1</Inspected> <Rejects>1</Rejects> <Defects>1</Defects> <Autoreject>0</Autoreject> <Sensor id=\"40\"> <Rejects>0</Rejects> <Defects>0</Defects> </Sensor> <Sensor id=\"41\"> <Rejects>1</Rejects> <Defects>1</Defects> <Counter id=\"1\" Nb=\"1\"/> </Sensor> <Sensor id=\"42\"> <Rejects>0</Rejects> <Defects>0</Defects> </Sensor> <Sensor id=\"46\"> <Rejects>1</Rejects> <Defects>1</Defects> <Counter id=\"1\" Nb=\"1\"/> <Counter id=\"2\" Nb=\"1\"/> </Sensor> </Mold></Machine> </Root> ";
    QString xmlstr1 = "<Root xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"\"> <Machine Id=\"MX\"/> </Root> ";
    parser.PaserDataToDataBase(xmlstr1, &err);
} 


using namespace std;
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    QApplication::setLibraryPaths(QStringList(QString(QCoreApplication::applicationDirPath()+"/QtPlugins/")));

    cout << "Program Starting..."<<endl;
    //testParserXmlData();

    DataCenter parser;
    QString err;
    if(!parser.Init(&err))
    {
        cout<<err.toLocal8Bit().constData()<<endl;
        system("pause");
        return 1;
    }

	//test w;
	//w.show();
	struct soap m_oSoap;
	// WebService调用对象
	class _ns1__Counts getMobileCodeObject;
	// WebService返回对象
	class _ns1__CountsResponse getMobileCodeResponseObject;
	// SOAP初始化
	soap_init(&m_oSoap);
	// 调用函数的参数赋值
    cout << "Program Init Complete"<<endl;
    while (true)
    {//每半个小时执行一次
        // 发送WebService请求，并获得返回结果
        int nResult = soap_call___ns1__Counts(&m_oSoap,NULL,NULL,&getMobileCodeObject,getMobileCodeResponseObject);
        // 操作成功
        if(SOAP_OK == nResult)
        {
            // 输出返回结果
            char* strResult = getMobileCodeResponseObject.CountsResult->__any;
            QString xmlstr = QString::fromLocal8Bit(strResult);
            QString err;
            if(!parser.PaserDataToDataBase(xmlstr, &err))
            {
                qDebug()<<"Parser Count Data To DataBase Error:"<<qPrintable(err);
                Sleep(1000*10);
                continue;
            }

        }else{
            Sleep(1000);
            continue;
        }

        //Sleep(1000*60*30);
        //Sleep(1000*60);
        Sleep(1000*10);
        //Sleep(1000);
    }
	
	/// 关闭SOAP
	soap_destroy(&m_oSoap);
	soap_end(&m_oSoap);
	soap_done(&m_oSoap);
	return a.exec();
}
