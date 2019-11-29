/********************************************************************
* @filename  : DataCenter.h
*
* @brief     : 本模块用于解析系统的webservice返回的XML字符串，并保存到数据库中
*	
* @author    : Mingbing.Guo
* @date      : 2019-11-12
* 
* @version   : V0.1

//以下为示例XML字符串，无换行的原字符串
//<Root xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="">
//    <Machine Id="MX">
//    <!-- 过检总数 -->
//    <Inspected>1</Inspected>
//    <!-- 发剔废信号数 -->
//    <Rejects>1</Rejects>
//    <!-- 实际剔废数 -->
//    <Defects>1</Defects>
//    <!-- 信号差值数 -->
//    <Autoreject>0</Autoreject>
//    <!-- 模号 0-99 可能有多个 -->
//    <Mold id="0">
//    <!-- 过检总数 | 发剔废信号数 | 实际剔废数 | 信号差值数 -->
//    <Inspected>1</Inspected>
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Autoreject>0</Autoreject>
//
//    <!-- 缺陷代码 | 检测个数 | 实际剔废个数 | 其他变量-->
//    <Sensor id="40">
//    <Rejects>0</Rejects>
//    <Defects>0</Defects>
//    </Sensor>
//    <Sensor id="41">
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Counter id="1" Nb="1" />
//    </Sensor>
//    <Sensor id="42">
//    <Rejects>0</Rejects>
//    <Defects>0</Defects>
//    </Sensor>
//    <Sensor id="46">
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Counter id="1" Nb="1" />
//    <Counter id="2" Nb="1" />
//    </Sensor>
//    </Mold>
//    </Machine>
//    </Root>
*********************************************************************/
#ifndef DATACANTER_20GCLXZDB2VN57N8_H_
#define DATACANTER_20GCLXZDB2VN57N8_H_

#include <QString>
#include <QDateTime>
#include "DataDef.hpp"


// 此类是从 DataCenter.dll 导出的
/**
    此类用于将传入的XML数据保存到运行目录的./Data/Satistics.db3中
    表结构定义请参考文档 解析数据库表结构.xlsx
*/

#include "datacenter_global.h"

#include <QObject>

class DATACENTER_EXPORT DataCenter: public QObject
{
    Q_OBJECT
public:
    DataCenter();
    ~DataCenter();

public slots:
    /**
    * @brief  :  Init 初始化， 内部会检查数据库是否创建，如果没有创建则会创建数据库
    *
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init(QString* err = nullptr);

    void Stop();

    /**
    * @brief  :  PaserDataToDataBase 将传入的XML格式的字符串解析后将数据保存到数据库中
    *
    * @param  :  const QString & xmldata XML格式的字符串，字符串格式见文件头注释部分
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool PaserDataToDataBase(const QString& xmldata, QString* err = nullptr);

    /**
    * @brief  :  signal_GetDataByTime 获取最新一条记录
    *
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    bool GetLastestRecord(Record& data, QString* err);

    /**
    * @brief  :  GetTimeInterval 获取时间间隔
    *
    * @return :  ETimeInterval
    * @retval :
    */
    ETimeInterval GetTimeInterval();

    /**
    * @brief  :  SetTimeInterval
    *
    * @param  :  ETimeInterval timeinterval
    * @return :  void
    * @retval :
    */
    void SetTimeInterval(ETimeInterval timeinterval);

    /**
    * @brief  :  signal_GetAllDate 获取当前记录的日期列表
    *
    * @param  :  QList<QDate> & lst
    * @return :  bool
    * @retval :
    */
    bool GetAllDate(QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay 获取指定日期的记录起始时间和结束时间列表
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    bool GetRecordListByDay(QDate date, QList<QTime>& stlst, QList<QTime>& etlst);
     
    /**
    * @brief  :  signal_GetDataByTime 按时间获取数据,相当于设置可设置数据来源,根据链接的不同来源的槽函数
    *
    * @param  :  QDateTime st   起始时间
    * @param  :  QDateTime end  结束时间
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    bool GetRecordByTime(QDateTime st, QDateTime end, Record& data);

private:
    class Impl;

    Impl* m_pimpl;

};

#endif//DATACANTER_20GCLXZDB2VN57N8_H_

