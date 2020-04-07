#ifndef DBOperationB_8WXOXPIVYL3XP0D0_H_
#define DBOperationB_8WXOXPIVYL3XP0D0_H_

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
class DBOperationB : public QObject
{
    Q_OBJECT
public:
    DBOperationB(QObject* parent = nullptr);
    virtual ~DBOperationB();

    /**
    * @brief  :  Init 数据库初始化，当数据库文件不存在时，会创建文件，然后创建表
    *                   解析策略文件
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    virtual bool Init( QString* err = nullptr);

    /**
    * @brief  :  SaveData 保存数据到数据库，每次都重新打开数据库，因为操作频率很小
    *
    * @param  :  const XmlData & data
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    virtual bool SaveData(const XmlData& data, QString* err = nullptr)=0;

    virtual void Start();
    virtual void Stop();
	
    /**
    * @brief  :  signal_GetDataByTime 获取最新一条记录
    *
	* @param type: 0:按时间间隔 1:按班次
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    virtual bool GetLastestRecord(int type, Record& data, QString* err)=0;
	
    /**
    * @brief  :  signal_GetAllDate 获取当前记录的日期列表
	*
	* @param type: 0:按时间间隔 1:按班次
    * @param  :  QList<QDate> & lst
    * @return :  bool
    */
    virtual bool GetAllDate(int type, QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay 获取指定日期的记录起始时间和结束时间列表
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    virtual bool GetRecordTimeListByDay(int type, QDate date, QList<QDateTime>& stlst, QList<QDateTime>& etlst);
     
    /**
    * @brief  :  signal_GetDataByTime 按时间获取数据,相当于设置可设置数据来源,根据链接的不同来源的槽函数
    *
    * @param  :  QDateTime st   起始时间
    * @param  :  QDateTime end  结束时间
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    virtual bool GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data)=0;

signals:
    void sig_start();
protected:
	/** checkSensorIDIsShouldSave
	 * @brief 检查指定ID缺陷信息是否需要保存
	 * @param id:缺陷id
	 */
    virtual bool checkSensorIDIsShouldSave(int id);
    /**
    * @brief  :  Init 初始化对应名字的数据库文件
    *
    * @param  :  QString filelname
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    virtual bool Init(QSharedPointer<QSqlDatabase> _db, QString filelname, QString* err = nullptr, bool isclassdb = false);
	virtual void DeleteOutdatedData(int outdatedays);

	virtual bool GetLastestShiftRecordEndTime(QDateTime& t, QString* err);
	virtual bool GetLatestRecordEndTime(QDateTime& t, QString* err);

	virtual bool GetLastestRecordEndTime(QSharedPointer<QSqlDatabase> db, QDateTime& t, QString* err);
	virtual bool GetOldestRecordStartTime(QSharedPointer<QSqlDatabase> db, QDateTime& t, QString* err);

	virtual bool DeleteRecordAfterTime(EOperDatabase type, QDateTime time);
	virtual bool GetOperDBParams(EOperDatabase type, QSharedPointer<QSqlDatabase>& _db, QSharedPointer<QMutex>& _db_mutex);
	virtual bool GetRecordDBParams(ERecordType type, QSharedPointer<QSqlDatabase>& _db, QSharedPointer<QMutex>& _db_mutex);
	virtual bool GetDBOperType(ERecordType type, EOperDatabase& optype);
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

#endif//DBOperationB_8WXOXPIVYL3XP0D0_H_

