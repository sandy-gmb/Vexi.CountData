#include <QApplication>
#include <iostream>

#include "datacenter.h"
#include "dataview.h"
#include "Core.hpp"
#include "Config.hpp"

#include "logger.h"

#include <QObject>
#include <QStringList>
#include <QThreadPool>

#include <QTextCodec>

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
    QTextCodec* code = QTextCodec::codecForName("system");
    QTextCodec::setCodecForTr(code);
    QTextCodec::setCodecForLocale(code);
    QTextCodec::setCodecForCStrings(code);

    QApplication::setLibraryPaths(QStringList(QString(QCoreApplication::applicationDirPath()+"/QtPlugins/")));
        //testParserXmlData();
    ELOGGER->SetLogLevel(EasyLog::LOG_INFO); 
    //ELOGGER->SetLogLevel(EasyLog::LOG_TRACE); 
    ELOGGER->SetPrint2StdOut(false);

    ELOGI( "Program Starting...");

    QString err;

    Config cfg;
    DataCenter da;
    Core core;
    int lang = cfg.GetSoftwareLanguage();
    DataView view(lang);

     QObject::connect(&da, SIGNAL(GetDataCenterConf(DataCenterConf&)), &cfg, SLOT(GetDataCenterConf(DataCenterConf&)));
     QObject::connect(&da, SIGNAL(SetGenerateRecordTimeInterval(int)), &cfg, SLOT(SetGenerateRecordTimeInterval(int)));
     QObject::connect(&core, SIGNAL(GetTimeOfObtainSrcData()), &cfg, SLOT(GetTimeOfObtainSrcData()));
     QObject::connect(&view, SIGNAL(GetWordsTranslationFilePath()), &cfg, SLOT(GetWordsTranslationFilePath()));

    QObject::connect(&core, SIGNAL(signal_PaserDataToDataBase(const QString& , QString*)), &da, SLOT(PaserDataToDataBase(const QString& , QString*)), Qt::DirectConnection);
    QObject::connect(&view, SIGNAL(signal_GetLastestRecord(Record& , QString*)), &da, SLOT(GetLastestRecord(Record& , QString*)));
    QObject::connect(&view, SIGNAL(signal_GetTimeInterval()), &da, SLOT(GetTimeInterval()));
    QObject::connect(&view, SIGNAL(signal_SetTimeInterval(ETimeInterval)), &da, SLOT(SetTimeInterval(ETimeInterval)));
    QObject::connect(&view, SIGNAL(signal_GetAllDate(QList<QDate>&)), &da, SLOT(GetAllDate(QList<QDate>&)));
    QObject::connect(&view, SIGNAL(signal_GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>&)), &da, SLOT(GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>&)));
    QObject::connect(&view, SIGNAL(signal_GetRecordByTime(QDateTime , QDateTime , Record&)), &da, SLOT(GetRecordByTime(QDateTime , QDateTime , Record&)));
    QObject::connect(&view, SIGNAL(closed()), &core, SLOT(stop()), Qt::QueuedConnection);
    QObject::connect(&view, SIGNAL(closed()), &da, SLOT(Stop()));

    if(!da.Init(&err))
    {
        cout<<err.toLocal8Bit().constData()<<endl;
        system("pause");
        return 1;
    }

    view.Init();

    core.start();
	view.show();
	
	return a.exec();
}
