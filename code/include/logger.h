/*
 * Easylog.h
 *
 *  Created on: 2018��12��7��
 *      Author: Administrator
 *      ֧�ַ�C++11,֧����־����֧����ʽ��־,֧��printf������Ҫ���壬ֻ��Ҫ����ͷ�ļ�
 *      ��лԭ����sollyu
 *      by zhushuo
 */

#ifndef EASYLOG_H_
#define EASYLOG_H_

#pragma once

//
//  EasyLog.h
//
//  Created by sollyu on 14/11/20.
//  Copyright (c) 2014�� sollyu. All rights reserved.
//

/**
	#include <iostream>
	#include "Easylog.h"

	ELOGI("i'm %s", "zhushuo");          // ��� INFO (ֻ�� LOGI �����ӡ������)
	ELOGE("I'm " << "sollyu");          // ��� ERROR (���ӡ������)
	ELOG_DEBUG("i'm %s", "sollyu");     // ��� DEBUG (���ӡ������)
	EasyLog::GetInstance()->WriteLog(EasyLog::LOG_DEBUG, "i'm %s", "sollyu");

 // ��������ִ�н��
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
#  define EASY_LOG_FILE_NAME			"normal.txt"   /** ��־���ļ��� �� ��������������ļ��б���Ŀ¼,��ʹ�ô���*/
#endif

#ifndef EASY_LOG_FILE_NAME_DATE
#  define EASY_LOG_FILE_NAME_DATE		1            /** 1��ʾʹ��������Ϊ�ļ��� */
#endif

#ifndef EASY_LOG_LINE_BUFF_SIZE
#  define EASY_LOG_LINE_BUFF_SIZE		1024            /** һ�е���󻺳� */
#endif

#ifndef EASY_LOG_DISABLE_LOG
#  define EASY_LOG_DISABLE_LOG          0               /** ��0��ʾ����LOG */
#endif


#ifndef EASY_LOG_COVER_LOG
#  define EASY_LOG_COVER_LOG          1               /** ��0��ʾ׷��д,����д��ʹ��������Ϊ�ļ���,ͬһ������ֻ��һ���ļ� */
#endif


#ifdef WIN32
#else
#   define  sprintf_s   sprintf
#   define  vsnprintf_s vsnprintf//�˴���Ϊ���õ���mingw��ֱ������vsnprintf���������ж�
#endif

/** д��־���� */
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

//! ���ٺ�
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
    /** ��־����*/
	enum LOG_LEVEL { LOG_TRACE = 0, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_ALARM,  LOG_FATAL };

public:
    /** ����ģʽ */
    static EasyLog * GetInstance() ;
    //void EasyLogDestroy(){delete this;}//���ã�EasyLog::GetInstance()->EasyLogDestroy();��������ã�destroy���ٴε��û������ÿ��д��־�Ѿ�flush�ˣ����Բ�̫��Ҫ�����Ҫ���ã��뱣֤�������

public:
    /** д��־���� */
	void WriteLog(LOG_LEVEL level, const char *pLogText, ...);

	void WriteLog(std::string logText, LOG_LEVEL level = LOG_ERROR);

    /** ���ú��� */

    /**
    * @brief  :  SetLogDir ������־Ŀ¼
    *
    * @param  :  std::string dir
    * @return :  void
    * @retval :
    */
    void SetLogDir(std::string dir);

    /**
    * @brief  :  SetPrint2StdOut �����Ƿ�ͬʱ����־��ӡ����׼���
    *
    * @param  :  bool isprint
    * @return :  void
    * @retval :
    */
    void SetPrint2StdOut(bool isprint);

    /**
    * @brief  :  SetFileMaxSize ������־�ļ���С
    *
    * @param  :  int size ��λM
    * @return :  void
    * @retval :
    */
    void SetFileMaxSize(int size);

    void SetLogLevel(LOG_LEVEL level);
private:
    EasyLog(void);
    virtual ~EasyLog(void);

    bool Init();

    //����ļ���С
    bool CheckFileSize();

    std::string GenerateFilePath();

    bool ComfirmFolderExists(std::string filepath);

private:
    /** д�ļ� */
    std::ofstream m_fileOut;
    std::string filename;               //��ǰʹ�õ��ļ���      ��׼��ʽΪ ./log/2019-11-18/08_10_22_01.txt
    std::string file_name_prefix;       //��ǰʹ�õ��ļ���ǰ׺  
    int index;                          //�ļ�����׺
    long long filesize;

    bool isinited ;

    LOG_LEVEL level;

    //���ں���Ϊ���뿪�ز������л�ʹ��,ÿ�ζ���Ҫ���±��� ��˽����ܻ�ı�ĺ��޸�Ϊ������������ú���

    std::string dir;
    bool isprint2stdout;
    int filemaxsize;
};

#define ELOGGER EasyLog::GetInstance()

#endif /* EASYLOG_H_ */
