#ifndef SAVEDATATODB_JQQ8207II27VSQ97_H_
#define SAVEDATATODB_JQQ8207II27VSQ97_H_

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

/************************************************************************/
/* 保存数据到数据库，此类的隐藏逻辑是会根据策略文件的策略存放指定范围的数据     */
/************************************************************************/
class DBOperation : public QObject
{
    Q_OBJECT
public:
    DBOperation(QObject* parent = nullptr);
    ~DBOperation();

    /**
    * @brief  :  Init 数据库初始化，当数据库文件不存在时，会创建文件，然后创建表
    *                   解析策略文件
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init( QString* err = nullptr);

    /**
    * @brief  :  SaveData 保存数据到数据库，每次都重新打开数据库，因为操作频率很小
    *
    * @param  :  const XmlData & data
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool SaveData(const XmlData& data, QString* err = nullptr);

    void Start();
    void Stop();
    
    /**
    * @brief GetTimeInterval 获取时间间隔
    *
    * @return:   ETimeInterval
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
    * @brief  :  signal_GetDataByTime 获取最新一条记录
    *
	* @param type: 0:按时间间隔 1:按班次
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    bool GetLastestRecord(int type, Record& data, QString* err);
	
    /**
    * @brief  :  signal_GetAllDate 获取当前记录的日期列表
	*
	* @param type: 0:按时间间隔 1:按班次
    * @param  :  QList<QDate> & lst
    * @return :  bool
    */
    bool GetAllDate(int type, QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay 获取指定日期的记录起始时间和结束时间列表
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    bool GetRecordTimeListByDay(int type, QDate date, QList<QTime>& stlst, QList<QTime>& etlst);
     
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

	void RecordConfigChanged();
	void OnDataConfChange(const DataCenterConf& cfg);
signals:
    void sig_start();
private:
	/** checkSensorIDIsShouldSave
	 * @brief 检查指定ID缺陷信息是否需要保存
	 * @param id:缺陷id
	 */
    bool checkSensorIDIsShouldSave(int id);
    /**
    * @brief  :  Init 初始化对应名字的数据库文件
    *
    * @param  :  QString filelname
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init(QSharedPointer<QSqlDatabase> _db, QString filelname, QString* err = nullptr, bool isclassdb = false);

	bool QueryRecordByTime(EOperDatabase type, QDateTime st, QDateTime et,QList<Record>& lst, QString* err);
	bool QueryMoldInfoByMainID(QSharedPointer<QSqlDatabase> db, int mainrowid, QList<MoldInfo>& moldinfo, QString* err);
	bool QuerySensorInfoByMoldID(QSharedPointer<QSqlDatabase> db, int moldid, QList<SensorInfo>& sensorinfo, QString* err);
	bool QuerySensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> db, int sensorid, QMap<int, int>& addinginfo, QString* err);

	bool SaveRecord(EOperDatabase type, const Record& lst, QString* err, bool isoutdb = false);
	bool SaveMoldInfoByMainID(QSharedPointer<QSqlDatabase> db, int mainrowid, const QList<MoldInfo>& moldinfo, QString* err);
	bool SaveSensorInfoByMoldID(QSharedPointer<QSqlDatabase> db, int moldrowid, const QList<SensorInfo>& sensorinfo, QString* err);
	bool SaveSensorAddingInfoBySensorID(QSharedPointer<QSqlDatabase> db, int sensorrowid, const QMap<int, int>& addinginfo, QString* err);
	
	bool QueryShiftRecordByClass(QSharedPointer<QSqlDatabase> _db, const QDate& d,int shift, Record record, QString* err);
	bool SaveShiftRecord(QSharedPointer<QSqlDatabase> _db, const Record& re, QString* err, bool isoutdb = false);

	bool GetLastestRecordEndTime(QSharedPointer<QSqlDatabase> db, QDateTime& t, QString* err);
	bool GetOldestRecordStartTime(QSharedPointer<QSqlDatabase> db, QDateTime& t, QString* err);

	/********************* process Generate Record ******************************************************************************************************/
	bool GetLatestRecordEndTime(QDateTime& t, QString* err);
	void DeleteOutdatedData(int outdatedays);

	bool GetLastestShiftRecordEndTime(QDateTime& t, QString* err);
	bool DeleteRecordAfterTime(EOperDatabase type, QDateTime time);

	bool GetOperDBParams(EOperDatabase type, QSharedPointer<QSqlDatabase>& _db, QSharedPointer<QMutex>& _db_mutex);
	bool GetRecordDBParams(ERecordType type, QSharedPointer<QSqlDatabase>& _db, QSharedPointer<QMutex>& _db_mutex);
	bool GetDBOperType(ERecordType type, EOperDatabase& optype);
	bool SaveRecordList(EOperDatabase type, QList<Record>& newlst, QString* err);

public:
    //策略数据
	StrategyConfig cfgStrategy;

	GenerateRecord* work;
    QThread* thd;            //线程对象 用于包裹GenerateRecord

	QSharedPointer<QSqlDatabase> db;        //连接时间间隔统计记录数据库的对象,使用一个对象,防止重复打开
	QSharedPointer<QMutex>  rtdb_mutex;
	QSharedPointer<QSqlDatabase> rtdb;      //连接实时数据库的对象,使用一个对象,防止重复打开
	QSharedPointer<QMutex>  db_mutex;
	QSharedPointer<QSqlDatabase> shiftdb;   //连接实时数据库的对象,使用一个对象,防止重复打开
	QSharedPointer<QMutex>  shiftdb_mutex;

	friend class GenerateRecord;
};

#endif//SAVEDATATODB_JQQ8207II27VSQ97_H_

