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
/* �������ݵ����ݿ⣬����������߼��ǻ���ݲ����ļ��Ĳ��Դ��ָ����Χ������     */
/************************************************************************/
class DBOperationB : public QObject
{
    Q_OBJECT
public:
    DBOperationB(QObject* parent = nullptr);
    virtual ~DBOperationB();

    /**
    * @brief  :  Init ���ݿ��ʼ���������ݿ��ļ�������ʱ���ᴴ���ļ���Ȼ�󴴽���
    *                   ���������ļ�
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    virtual bool Init( QString* err = nullptr);

    /**
    * @brief  :  SaveData �������ݵ����ݿ⣬ÿ�ζ����´����ݿ⣬��Ϊ����Ƶ�ʺ�С
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
    * @brief  :  signal_GetDataByTime ��ȡ����һ����¼
    *
    * @param type: 0:��ʱ���� 1:�����
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    virtual bool GetLastestRecord(int type, Record& data, QString* err)=0;
    
    /**
    * @brief  :  signal_GetAllDate ��ȡ��ǰ��¼�������б�
    *
    * @param type: 0:��ʱ���� 1:�����
    * @param  :  QList<QDate> & lst
    * @return :  bool
    */
    virtual bool GetAllDate(int type, QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay ��ȡָ�����ڵļ�¼��ʼʱ��ͽ���ʱ���б�
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    virtual bool GetRecordTimeListByDay(int type, QDate date, QList<QDateTime>& stlst, QList<QDateTime>& etlst);
     
    /**
    * @brief  :  signal_GetDataByTime ��ʱ���ȡ����,�൱�����ÿ�����������Դ,�������ӵĲ�ͬ��Դ�Ĳۺ���
    *
    * @param  :  QDateTime st   ��ʼʱ��
    * @param  :  QDateTime end  ����ʱ��
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    virtual bool GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data)=0;

signals:
    void sig_start();
protected:
    /** checkSensorIDIsShouldSave
     * @brief ���ָ��IDȱ����Ϣ�Ƿ���Ҫ����
     * @param id:ȱ��id
     */
    virtual bool checkSensorIDIsShouldSave(QString id);
    /**
    * @brief  :  Init ��ʼ����Ӧ���ֵ����ݿ��ļ�
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

#endif//DBOperationB_8WXOXPIVYL3XP0D0_H_

