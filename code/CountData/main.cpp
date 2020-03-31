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


#ifdef WIN32
#include <io.h>		// windows
#include <Windows.h>
#pragma warning(disable:4091)
#include <DbgHelp.h>
#else
#include <unistd.h>		// linux
#endif
#pragma comment(lib,"dbghelp.lib")    // dump


using namespace std;

static LONG WINAPI pfnUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	if (IsDebuggerPresent())
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	HMODULE hDbgHelp = LoadLibrary(L"dbghelp.dll");
	if (NULL == hDbgHelp)
	{
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	SYSTEMTIME stSysTime;
	memset(&stSysTime, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&stSysTime);

	char szFile[MAX_PATH] = { 0 };		//根据字符集，有时候可能为WCHAR
	// 判断目录是否存在
	if (ENOENT == _access_s(".\\dump", 0))
	{
		CreateDirectoryA(".\\dump", NULL);
	}

	sprintf_s(szFile, ".\\dump\\%0.4d-%0.2d-%0.2d-%0.2d-%0.2d-%0.2d-%0.3d.dmp", \
		stSysTime.wYear, stSysTime.wMonth, stSysTime.wDay, stSysTime.wHour, \
		stSysTime.wMinute, stSysTime.wSecond, stSysTime.wMilliseconds);

	HANDLE hFile = CreateFileA(szFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, \
		0, CREATE_ALWAYS, 0, 0);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		MINIDUMP_EXCEPTION_INFORMATION objExInfo;
		objExInfo.ThreadId = ::GetCurrentThreadId();
		objExInfo.ExceptionPointers = pExceptionInfo;
		objExInfo.ClientPointers = NULL;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, \
			MiniDumpWithDataSegs, (pExceptionInfo ? &objExInfo : NULL), NULL, NULL);
		CloseHandle(hFile);
	}

	FreeLibrary(hDbgHelp);

	return EXCEPTION_EXECUTE_HANDLER;
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

	//ELOGGER->SetLogLevel(EasyLog::LOG_TRACE); 
	ELOGGER->SetPrint2StdOut(false);

	ELOGI( "Program Starting...");

	SetUnhandledExceptionFilter(pfnUnhandledExceptionFilter);
	QString err;

	Config cfg;
	ELOGGER->SetLogLevel((EasyLog::LOG_LEVEL)cfg.GetConfigLogLevel());

	DataCenter da;
	Core core;

	int lang = cfg.GetSoftwareLanguage();
	DataView view(lang);

	QObject::connect(&da, SIGNAL(GetDataCenterConf(DataCenterConf&)), &cfg, SLOT(GetDataCenterConf(DataCenterConf&)));
	QObject::connect(&da, SIGNAL(SetGenerateRecordTimeInterval(int)), &cfg, SLOT(SetGenerateRecordTimeInterval(int)));
	QObject::connect(&core, SIGNAL(signal_GetCoreConf(CoreConf& )), &cfg, SLOT(GetCoreConf(CoreConf& )), Qt::DirectConnection);
	QObject::connect(&view, SIGNAL(GetWordsTranslationFilePath()), &cfg, SLOT(GetWordsTranslationFilePath()));

	QObject::connect(&core, SIGNAL(signal_PaserDataToDataBase(const QString& , QString*)), &da, SLOT(PaserDataToDataBase(const QString& , QString*)), Qt::DirectConnection);
	QObject::connect(&view, SIGNAL(signal_GetLastestRecord(int , Record& , QString*)), &da, SLOT(GetLastestRecord(int , Record& , QString*)));
	QObject::connect(&view, SIGNAL(signal_GetTimeInterval()), &da, SLOT(GetTimeInterval()));
	QObject::connect(&view, SIGNAL(signal_SetTimeInterval(ETimeInterval)), &da, SLOT(SetTimeInterval(ETimeInterval)));
	QObject::connect(&view, SIGNAL(signal_GetAllDate(int , QList<QDate>&)), &da, SLOT(GetAllDate(int , QList<QDate>&)));
	QObject::connect(&view, SIGNAL(signal_GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>&)), &da, SLOT(GetRecordListByDay(QDate , QList<QTime>& , QList<QTime>&)));
	QObject::connect(&view, SIGNAL(signal_GetRecordByTime(QDateTime , QDateTime , Record&)), &da, SLOT(GetRecordByTime(QDateTime , QDateTime , Record&)));
	QObject::connect(&view, SIGNAL(closed()), &core, SLOT(stop()), Qt::QueuedConnection);
	QObject::connect(&view, SIGNAL(closed()), &da, SLOT(Stop()));

	if(!da.Init(&err))
	{
		ELOGE("%s", qPrintable(err));
		system("pause");
		return 1;
	}

	view.Init();

	core.start();
	view.show();

	return a.exec();
}
