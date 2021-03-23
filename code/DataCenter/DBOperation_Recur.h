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
* �������ݵ����ݿ⣬�ڲ����õݹ��ȡ�ͱ���,���ڵ�ǰʹ�����ݿ���SQLite ��˶�ȡ�ͱ������ݿ��Ч�ʺܵ�     
*********************************************************************************************************/
class DBOperation_Recur : public DBOperationB
{
    Q_OBJECT
public:
    DBOperation_Recur(QObject* parent = nullptr);
    virtual ~DBOperation_Recur();

    /**
    * @brief  :  SaveData �������ݵ����ݿ⣬ÿ�ζ����´����ݿ⣬��Ϊ����Ƶ�ʺ�С
    *
    * @param  :  const XmlData & data
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    virtual bool SaveData(const XmlData& data, QString* err = nullptr);

    /**
    * @brief  :  signal_GetDataByTime ��ȡ����һ����¼
    *
    * @param type: 0:��ʱ���� 1:�����
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    virtual bool GetLastestRecord(int type, Record& data, QString* err);
     
    /**
    * @brief  :  signal_GetDataByTime ��ʱ���ȡ����,�൱�����ÿ�����������Դ,�������ӵĲ�ͬ��Դ�Ĳۺ���
    *
    * @param  :  QDateTime st   ��ʼʱ��
    * @param  :  QDateTime end  ����ʱ��
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
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

