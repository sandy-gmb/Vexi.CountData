#ifndef SAVEDATATODB_JQQ8207II27VSQ97_H_
#define SAVEDATATODB_JQQ8207II27VSQ97_H_

#include "DBOperationB.h"
#include <QObject>
#include "DataDef.hpp"

#include <QList>
#include <QSet>
#include <QDateTime>

#include <QSqlDatabase>
#include <QMutex>
#include <QSharedPointer>

#include "GenerateRecord.h"
#include "ConfigDef.hpp"

/************************************************************************/
/* 保存数据到数据库，此类的隐藏逻辑是会根据策略文件的策略存放指定范围的数据     */
/************************************************************************/
class DBOperation : public DBOperationB
{
    Q_OBJECT
public:
    DBOperation(QObject* parent = nullptr);
    virtual ~DBOperation();


    /**
    * @brief  :  SaveData 保存数据到数据库，每次都重新打开数据库，因为操作频率很小
    *
    * @param  :  const XmlData & data
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool SaveData(const XmlData& data, QString* err = nullptr);

    /**
    * @brief  :  signal_GetDataByTime 获取最新一条记录
    *
	* @param type: 0:按时间间隔 1:按班次
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    bool GetLastestRecord(int type, Record& data, QString* err);
	
    /**
    * @brief  :  signal_GetDataByTime 按时间获取数据,相当于设置可设置数据来源,根据链接的不同来源的槽函数
    *
    * @param  :  QDateTime st   起始时间
    * @param  :  QDateTime end  结束时间
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    bool GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data);

protected:

	bool QueryRecordByTime(EOperDatabase type, QDateTime st, QDateTime et,QList<Record>& lst, QString* err);
	bool QueryMoldInfoByMainID(QSharedPointer<QSqlDatabase> db, const QSet<int>& mainrowidlst,QMap<int, QList<MoldInfo> >& moldinfo, QSet<int>& moldrowidlst, QString* err);
	bool QuerySensorInfoByMoldID(QSharedPointer<QSqlDatabase> db,  const QSet<int>& moldrowidlst,QMap<int, QList<SensorInfo> >&  sensorinfo, QSet<int>& sensorrowidlst, QString* err);
	bool QuerySensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> db,  const QSet<int>& sensorrowidlst, QMap<int, QMap<QString, int> >& addinginfo, QString* err);

	bool SaveRecord(EOperDatabase type, const Record& lst, QString* err, bool isoutdb = false);
	bool SaveMoldInfoByMainID(QSharedPointer<QSqlDatabase> db, int mainrowid, const QList<MoldInfo>& moldinfo, QString* err);
	bool SaveSensorInfoByMoldID(QSharedPointer<QSqlDatabase> db, int moldrowid, const QList<SensorInfo>& sensorinfo, QString* err);
	bool SaveSensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> db, int sensorrowid, const QMap<QString, int>& addinginfo, QString* err);
	
	bool SaveShiftRecord(QSharedPointer<QSqlDatabase> _db, const Record& re, QString* err, bool isoutdb = false);
	/********************* process Generate Record ******************************************************************************************************/
	bool SaveRecordList(EOperDatabase type, QList<Record>& newlst, QString* err);

public:
	friend class GenerateRecord;
};

#endif//SAVEDATATODB_JQQ8207II27VSQ97_H_

