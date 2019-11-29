#include <QApplication>
#include <iostream>

#include "datacenter.h"
#include "dataview.h"
#include "Core.hpp"

#include "logger.h"

#include <QObject>
#include <QStringList>
#include <QThreadPool>

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

    ELOGGER->SetLogLevel(EasyLog::LOG_INFO); 
    ELOGGER->SetPrint2StdOut(false);

    ELOGI( "Program Starting...");
    //testParserXmlData();

    DataCenter parser;
    QString err;
    if(!parser.Init(&err))
    {
        cout<<err.toLocal8Bit().constData()<<endl;
        system("pause");
        return 1;
    }
    Core core;
    DataView view(&core);


    QObject::connect(&core, SIGNAL(signal_PaserDataToDataBase(const QString& , QString*)), &parser, SLOT(PaserDataToDataBase(const QString& , QString*)), Qt::DirectConnection);

    QObject::connect(&view, SIGNAL(signal_GetLastestRecord(Record& , QString*)), &parser, SLOT(GetLastestRecord(Record& , QString*)));
    QObject::connect(&view, SIGNAL(signal_GetTimeInterval()), &parser, SLOT(GetTimeInterval()));
    QObject::connect(&view, SIGNAL(signal_SetTimeInterval(ETimeInterval)), &parser, SLOT(SetTimeInterval(ETimeInterval)));
    QObject::connect(&view, SIGNAL(signal_GetAllDate(QList<QDate>&)), &parser, SLOT(GetAllDate(QList<QDate>&)));
    QObject::connect(&view, SIGNAL(signal_GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>&)), &parser, SLOT(GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>&)));
    QObject::connect(&view, SIGNAL(signal_GetRecordByTime(QDateTime , QDateTime , Record&)), &parser, SLOT(GetRecordByTime(QDateTime , QDateTime , Record&)));
    QObject::connect(&view, SIGNAL(closed()), &core, SLOT(stop()), Qt::QueuedConnection);
    QObject::connect(&view, SIGNAL(closed()), &parser, SLOT(Stop()));


    core.start();
	view.show();
	
	return a.exec();
}
