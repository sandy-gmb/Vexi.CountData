/*
 * Easylog.h
 *
 *  Created on: 2018年12月7日
 *      Author: Administrator
 *      支持非C++11,支持日志级别，支持流式日志,支持printf，不需要定义，只需要包含头文件
 *      感谢原作者sollyu
 *      by zhushuo
 */

#ifndef EASYLOG_H_
#define EASYLOG_H_

#pragma once

//
//  EasyLog.h
//
//  Created by sollyu on 14/11/20.
//  Copyright (c) 2014年 sollyu. All rights reserved.
//

/**
	#include <iostream>
	#include "Easylog.h"

	ELOGI("i'm %s", "zhushuo");          // 输出 INFO (只有 LOGI 不会打印所在行)
	ELOGE("I'm " << "sollyu");          // 输出 ERROR (会打印所在行)
	ELOG_DEBUG("i'm %s", "sollyu");     // 输出 DEBUG (会打印所在行)
	EasyLog::GetInstance()->WriteLog(EasyLog::LOG_DEBUG, "i'm %s", "sollyu");

 // 上面代码的执行结果
[2018.12.09] [19:48:41] [LOG_INFO ] ------------------ LOG SYSTEM START ------------------

[2018.12.09] [19:48:41] [LOG_INFO ] i'm zhushuo
[2018.12.09] [19:48:41] [LOG_ERROR] I'm sollyu (..\src\Easylog.cpp : main : 16 )
[2018.12.09] [19:48:41] [LOG_DEBUG] i'm sollyu (..\src\Easylog.cpp : main : 17 )
[2018.12.09] [19:48:41] [LOG_DEBUG] i'm sollyu

 */
 
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <functional>
#include <iomanip>
#include <stdarg.h>
#include <time.h>


#ifndef EASY_LOG_FILE_NAME
#  define EASY_LOG_FILE_NAME			"normal.txt"   /** 日志的文件名 ， 如果不以用日期文件夹保存目录,则使用此名*/
#endif

#ifndef EASY_LOG_FILE_NAME_DATE
#  define EASY_LOG_FILE_NAME_DATE		1            /** 1表示使用日期作为文件名 */
#endif

#ifndef EASY_LOG_LINE_BUFF_SIZE
#  define EASY_LOG_LINE_BUFF_SIZE		1024            /** 一行的最大缓冲 */
#endif

#ifndef EASY_LOG_DISABLE_LOG
#  define EASY_LOG_DISABLE_LOG          0               /** 非0表示禁用LOG */
#endif


#ifndef EASY_LOG_COVER_LOG
#  define EASY_LOG_COVER_LOG          1               /** 非0表示追加写,覆盖写则使用日期作为文件名,同一个日期只有一个文件 */
#endif


#ifdef WIN32
#else
#   define  sprintf_s   sprintf
#   define  vsnprintf_s vsnprintf//此处因为我用的是mingw，直接用了vsnprintf，请自行判断
#endif

/** 写日志方法 */
#define EWRITE_LOG(LEVEL, FMT, ...) \
{ \
    std::stringstream ss; \
    ss << FMT; \
    if (LEVEL != EasyLog::LOG_INFO) \
    { \
        ss << " (" << __FILE__ << " : " << __FUNCTION__ << " : " << __LINE__ << " )"; \
    } \
    EasyLog::GetInstance()->WriteLog(LEVEL, ss.str().c_str(), ##__VA_ARGS__); \
}

//! 快速宏
#define ELOG_TRACE(FMT , ...) EWRITE_LOG(EasyLog::LOG_TRACE, FMT, ##__VA_ARGS__)
#define ELOG_DEBUG(FMT , ...) EWRITE_LOG(EasyLog::LOG_DEBUG, FMT, ##__VA_ARGS__)
#define ELOG_INFO(FMT  , ...) EWRITE_LOG(EasyLog::LOG_INFO , FMT, ##__VA_ARGS__)
#define ELOG_WARN(FMT  , ...) EWRITE_LOG(EasyLog::LOG_WARN , FMT, ##__VA_ARGS__)
#define ELOG_ERROR(FMT , ...) EWRITE_LOG(EasyLog::LOG_ERROR, FMT, ##__VA_ARGS__)
#define ELOG_ALARM(FMT , ...) EWRITE_LOG(EasyLog::LOG_ALARM, FMT, ##__VA_ARGS__)
#define ELOG_FATAL(FMT , ...) EWRITE_LOG(EasyLog::LOG_FATAL, FMT, ##__VA_ARGS__)

#define ELOGT( FMT , ... ) ELOG_TRACE(FMT, ##__VA_ARGS__)
#define ELOGD( FMT , ... ) ELOG_DEBUG(FMT, ##__VA_ARGS__)
#define ELOGI( FMT , ... ) ELOG_INFO (FMT, ##__VA_ARGS__)
#define ELOGW( FMT , ... ) ELOG_WARN (FMT, ##__VA_ARGS__)
#define ELOGE( FMT , ... ) ELOG_ERROR(FMT, ##__VA_ARGS__)
#define ELOGA( FMT , ... ) ELOG_ALARM(FMT, ##__VA_ARGS__)
#define ELOGF( FMT , ... ) ELOG_FATAL(FMT, ##__VA_ARGS__)

#include "logger_global.h"

class LOGGER_API EasyLog
{
public:
    /** 日志级别*/
	enum LOG_LEVEL { LOG_TRACE = 0, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_ALARM,  LOG_FATAL };

public:
    /** 单例模式 */
    static EasyLog * GetInstance() ;
    //void EasyLogDestroy(){delete this;}//调用：EasyLog::GetInstance()->EasyLogDestroy();不建议调用，destroy后再次调用会崩溃，每次写日志已经flush了，所以不太需要，如果要调用，请保证在最后用

public:
    /** 写日志操作 */
	void WriteLog(LOG_LEVEL level, const char *pLogText, ...);

	void WriteLog(std::string logText, LOG_LEVEL level = LOG_ERROR);

    /** 设置函数 */

    /**
    * @brief  :  SetLogDir 设置日志目录
    *
    * @param  :  std::string dir
    * @return :  void
    * @retval :
    */
    void SetLogDir(std::string dir);

    /**
    * @brief  :  SetPrint2StdOut 设置是否同时将日志打印到标准输出
    *
    * @param  :  bool isprint
    * @return :  void
    * @retval :
    */
    void SetPrint2StdOut(bool isprint);

    /**
    * @brief  :  SetFileMaxSize 设置日志文件大小
    *
    * @param  :  int size 单位M
    * @return :  void
    * @retval :
    */
    void SetFileMaxSize(int size);

    void SetLogLevel(LOG_LEVEL level);
private:
    EasyLog(void);
    virtual ~EasyLog(void);

    bool Init();

    //检查文件大小
    bool CheckFileSize();

    std::string GenerateFilePath();

    bool ComfirmFolderExists(std::string filepath);

private:
    /** 写文件 */
    std::ofstream m_fileOut;
    std::string filename;               //当前使用的文件名      标准样式为 ./log/2019-11-18/08_10_22_01.txt
    std::string file_name_prefix;       //当前使用的文件名前缀  
    int index;                          //文件名后缀
    long long filesize;

    bool isinited ;

    LOG_LEVEL level;

    //由于宏作为编译开关不方便切换使用,每次都需要重新编译 因此将可能会改变的宏修改为变量并添加设置函数

    std::string dir;
    bool isprint2stdout;
    int filemaxsize;
};

#define ELOGGER EasyLog::GetInstance()

#endif /* EASYLOG_H_ */
