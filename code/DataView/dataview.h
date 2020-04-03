/********************************************************************
* @filename  : dataview.h
*
* @brief     : 此文件用于将数据显示到界面上
*	
* @author    : Guo.Mingbing
* @date      : 2019/11/25
* 
* @version   : V0.1
*********************************************************************/
#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "dataview_global.h"

#include <QObject>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QList>

#include "DataDef.hpp"
#include "ConfigDef.hpp"

class DATAVIEW_EXPORT DataView:public QObject
{
    Q_OBJECT
public:
    DataView( QObject* parent = nullptr);
    ~DataView();

public slots:
    void Init();
    void show();

	void OnLanguageChange(int lang);
	void OnChangeUI(EUISelection sel);
	void OnRecordShowChanged(int show);
signals:
	/****************  Config **********************************************************/
	void signal_GetAllConfig(AllConfig& cfg);
	void signal_SetAllConfig(const AllConfig& cfg);
	void signal_GetWordsTranslation(QMap<int, QString>& , QMap<int, QString>&);
	int signal_GetTimeInterval();


	/****************  DataCenter ******************************************************/
    /**
    * @brief  :  signal_GetDataByTime 获取最新一条记录
    *
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    bool signal_GetLastestRecord(int type, Record& data, QString* err);

    /**
    * @brief  :  signal_GetAllDate 获取当前记录的日期列表
    *
    * @param  :  QList<QDate> & lst
    * @return :  bool
    * @retval :
    */
    bool signal_GetAllDate(int type, QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay 获取指定日期的记录起始时间和结束时间列表
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    bool signal_GetRecordListByDay(int type, QDate date, QList<QTime>& stlst, QList<QTime>& etlst);
     
    /**
    * @brief  :  signal_GetDataByTime 按时间获取数据,相当于设置可设置数据来源,根据链接的不同来源的槽函数
    *
    * @param  :  QDateTime st   起始时间
    * @param  :  QDateTime end  结束时间
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
	bool signal_GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data);


	/****************  Self Defined ******************************************************/
    void closed();

private:
    class Impl;
    
    Impl* pimpl;

	friend class MainUI;
	friend class QueryWidget;
	friend class SettingWidget;

};

#endif // DATAVIEW_H
