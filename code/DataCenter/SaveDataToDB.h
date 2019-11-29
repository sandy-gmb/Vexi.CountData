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

/************************************************************************/
/* �������ݵ����ݿ⣬����������߼��ǻ���ݲ����ļ��Ĳ��Դ��ָ����Χ������     */
/************************************************************************/
class SaveDataToDB : public QObject
{
    Q_OBJECT
public:
    SaveDataToDB(QObject* parent = nullptr);
    ~SaveDataToDB();

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
    * @brief  :  signal_GetDataByTime ��ȡ����һ����¼
    *
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool GetLastestRecord(Record& data, QString* err);

    /**
    * @brief  :  GetTimeInterval ��ȡʱ����
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
    * @brief  :  signal_GetAllDate ��ȡ��ǰ��¼�������б�
    *
    * @param  :  QList<QDate> & lst
    * @return :  bool
    * @retval :
    */
    bool GetAllDate(QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay ��ȡָ�����ڵļ�¼��ʼʱ��ͽ���ʱ���б�
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    bool GetRecordListByDay(QDate date, QList<QTime>& stlst, QList<QTime>& etlst);
     
    /**
    * @brief  :  signal_GetDataByTime ��ʱ���ȡ����,�൱�����ÿ�����������Դ,�������ӵĲ�ͬ��Դ�Ĳۺ���
    *
    * @param  :  QDateTime st   ��ʼʱ��
    * @param  :  QDateTime end  ����ʱ��
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool GetRecordByTime(QDateTime st, QDateTime end, Record& data);

signals:
    void sig_start();
private:

    /**
    * @brief  :  ParserStrategyFile ���������ļ���ÿ��ִ��ǰ���һ������ļ��иĶ����ٽ���һ��
    *
    * @param  :  bool first
    * @return :  void
    * @retval :
    */
    void ParserStrategyFile(bool first = false);

    bool SaveStrategyData(QString* err = nullptr);

    bool checkSensorIDIsShouldSave(int id);
    /**
    * @brief  :  Init ��ʼ����Ӧ���ֵ����ݿ��ļ�
    *
    * @param  :  QString filelname
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init(QString filelname, QString* err = nullptr);

public:
    //��������
    bool strategy_mode;     //����ģʽ true��ʾ ������ false��ʾ������
    QList<int> sensorids;   //����Ӱ���ȱ��ID�б�
    QDateTime  lastmodify;  //�����ļ�������޸�ʱ��

    ETimeInterval eti;      //ʱ����,�����ڶ೤ʱ������һ��ͳ�Ƽ�¼

    bool m_islogevery_data; //�Ƿ񱣴�������־

    GenerateRecord* work;
    QThread* thd;            //�̶߳��� ���ڰ���GenerateRecord

    QSqlDatabase db;        //ʹ��һ������,��ֹ�ظ���
    QSharedPointer<QMutex>  rtdb_mutex;
    QSharedPointer<QMutex>  db_mutex;
    
};

#endif//SAVEDATATODB_JQQ8207II27VSQ97_H_

