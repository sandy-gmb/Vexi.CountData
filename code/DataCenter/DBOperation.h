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
/* �������ݵ����ݿ⣬����������߼��ǻ���ݲ����ļ��Ĳ��Դ��ָ����Χ������     */
/************************************************************************/
class DBOperation : public DBOperationB
{
    Q_OBJECT
public:
    DBOperation(QObject* parent = nullptr);
    virtual ~DBOperation();


    /**
    * @brief  :  SaveData �������ݵ����ݿ⣬ÿ�ζ����´����ݿ⣬��Ϊ����Ƶ�ʺ�С
    *
    * @param  :  const XmlData & data
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool SaveData(const XmlData& data, QString* err = nullptr);

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
    * @brief  :  signal_GetDataByTime ��ʱ���ȡ����,�൱�����ÿ�����������Դ,�������ӵĲ�ͬ��Դ�Ĳۺ���
    *
    * @param  :  QDateTime st   ��ʼʱ��
    * @param  :  QDateTime end  ����ʱ��
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
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

