#ifndef DBOperation_Recur_NGSIT5JNI5HKDVDV_H_
#define DBOperation_Recur_NGSIT5JNI5HKDVDV_H_

#include "DBOperationB.h"

#include <QObject>
#include "DataDef.hpp"

#include <QList>
#include <QDateTime>

#include <QThread>
#include <QSqlDatabase>
#include <QMutex>
#include <QSharedPointer>

#include "GenerateRecord.h"
#include "ConfigDef.hpp"

/********************************************************************************************************  
* 保存数据到数据库，内部采用递归读取和保存,由于当前使用数据库是SQLite 因此读取和保存数据库的效率很低     
*********************************************************************************************************/
class DBOperation_Recur : public DBOperationB
{
    Q_OBJECT
public:
    DBOperation_Recur(QObject* parent = nullptr);
    virtual ~DBOperation_Recur();

    /**
    * @brief  :  SaveData 保存数据到数据库，每次都重新打开数据库，因为操作频率很小
    *
    * @param  :  const XmlData & data
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    virtual bool SaveData(const XmlData& data, QString* err = nullptr);

    /**
    * @brief  :  signal_GetDataByTime 获取最新一条记录
    *
    * @param type: 0:按时间间隔 1:按班次
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    virtual bool GetLastestRecord(int type, Record& data, QString* err);
     
    /**
    * @brief  :  signal_GetDataByTime 按时间获取数据,相当于设置可设置数据来源,根据链接的不同来源的槽函数
    *
    * @param  :  QDateTime st   起始时间
    * @param  :  QDateTime end  结束时间
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    virtual bool GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data);

protected:

    virtual bool QueryRecordByTime(EOperDatabase type, QDateTime st, QDateTime et,QList<Record>& lst, QString* err);
    virtual bool QueryMoldInfoByMainID(QSharedPointer<QSqlDatabase> db, int mainrowid, QList<MoldInfo>& moldinfo, QString* err);
    virtual bool QuerySensorInfoByMoldID(QSharedPointer<QSqlDatabase> db, int moldid, QList<SensorInfo>& sensorinfo, QString* err);
    virtual bool QuerySensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> db, int sensorid, QMap<QString, int>& addinginfo, QString* err);

    virtual bool SaveRecord(EOperDatabase type, const Record& lst, QString* err, bool isoutdb = false);
    virtual bool SaveMoldInfoByMainID(QSharedPointer<QSqlDatabase> db, int mainrowid, const QList<MoldInfo>& moldinfo, QString* err);
    virtual bool SaveSensorInfoByMoldID(QSharedPointer<QSqlDatabase> db, int moldrowid, const QList<SensorInfo>& sensorinfo, QString* err);
    virtual bool SaveSensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> db, int sensorrowid, const QMap<QString, int>& addinginfo, QString* err);
    
    virtual bool SaveShiftRecord(QSharedPointer<QSqlDatabase> _db, const Record& re, QString* err, bool isoutdb = false);

    /********************* process Generate Record ******************************************************************************************************/
    
    virtual bool SaveRecordList(EOperDatabase type, QList<Record>& newlst, QString* err);

    friend class GenerateRecord;
};

#endif//DBOperation_Recur_NGSIT5JNI5HKDVDV_H_

