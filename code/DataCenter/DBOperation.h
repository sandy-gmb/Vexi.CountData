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
/* �������ݵ����ݿ⣬����������߼��ǻ���ݲ����ļ��Ĳ��Դ��ָ����Χ������     */
/************************************************************************/
class DBOperation : public QObject
{
    Q_OBJECT
public:
    DBOperation(QObject* parent = nullptr);
    ~DBOperation();

    /**
    * @brief  :  Init ���ݿ��ʼ���������ݿ��ļ�������ʱ���ᴴ���ļ���Ȼ�󴴽���
    *                   ���������ļ�
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init( QString* err = nullptr);

    /**
    * @brief  :  SaveData �������ݵ����ݿ⣬ÿ�ζ����´����ݿ⣬��Ϊ����Ƶ�ʺ�С
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
    * @brief GetTimeInterval ��ȡʱ����
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
    * @brief  :  signal_GetDataByTime ��ȡ����һ����¼
    *
	* @param type: 0:��ʱ���� 1:�����
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool GetLastestRecord(int type, Record& data, QString* err);
	
    /**
    * @brief  :  signal_GetAllDate ��ȡ��ǰ��¼�������б�
	*
	* @param type: 0:��ʱ���� 1:�����
    * @param  :  QList<QDate> & lst
    * @return :  bool
    */
    bool GetAllDate(int type, QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay ��ȡָ�����ڵļ�¼��ʼʱ��ͽ���ʱ���б�
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    bool GetRecordTimeListByDay(int type, QDate date, QList<QTime>& stlst, QList<QTime>& etlst);
     
    /**
    * @brief  :  signal_GetDataByTime ��ʱ���ȡ����,�൱�����ÿ�����������Դ,�������ӵĲ�ͬ��Դ�Ĳۺ���
    *
    * @param  :  QDateTime st   ��ʼʱ��
    * @param  :  QDateTime end  ����ʱ��
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data);

	void RecordConfigChanged();
	void OnDataConfChange(const DataCenterConf& cfg);
signals:
    void sig_start();
private:
	/** checkSensorIDIsShouldSave
	 * @brief ���ָ��IDȱ����Ϣ�Ƿ���Ҫ����
	 * @param id:ȱ��id
	 */
    bool checkSensorIDIsShouldSave(int id);
    /**
    * @brief  :  Init ��ʼ����Ӧ���ֵ����ݿ��ļ�
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
    //��������
	StrategyConfig cfgStrategy;

	GenerateRecord* work;
    QThread* thd;            //�̶߳��� ���ڰ���GenerateRecord

	QSharedPointer<QSqlDatabase> db;        //����ʱ����ͳ�Ƽ�¼���ݿ�Ķ���,ʹ��һ������,��ֹ�ظ���
	QSharedPointer<QMutex>  rtdb_mutex;
	QSharedPointer<QSqlDatabase> rtdb;      //����ʵʱ���ݿ�Ķ���,ʹ��һ������,��ֹ�ظ���
	QSharedPointer<QMutex>  db_mutex;
	QSharedPointer<QSqlDatabase> shiftdb;   //����ʵʱ���ݿ�Ķ���,ʹ��һ������,��ֹ�ظ���
	QSharedPointer<QMutex>  shiftdb_mutex;

	friend class GenerateRecord;
};

#endif//SAVEDATATODB_JQQ8207II27VSQ97_H_

