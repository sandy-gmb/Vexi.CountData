/********************************************************************
* @filename  : DataCenter.h
*
* @brief     : ��ģ�����ڽ���ϵͳ��webservice���ص�XML�ַ����������浽���ݿ���
*	
* @author    : Mingbing.Guo
* @date      : 2019-11-12
* 
* @version   : V0.1

//����Ϊʾ��XML�ַ������޻��е�ԭ�ַ���
//<Root xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="">
//    <Machine Id="MX">
//    <!-- �������� -->
//    <Inspected>1</Inspected>
//    <!-- ���޷��ź��� -->
//    <Rejects>1</Rejects>
//    <!-- ʵ���޷��� -->
//    <Defects>1</Defects>
//    <!-- �źŲ�ֵ�� -->
//    <Autoreject>0</Autoreject>
//    <!-- ģ�� 0-99 �����ж�� -->
//    <Mold id="0">
//    <!-- �������� | ���޷��ź��� | ʵ���޷��� | �źŲ�ֵ�� -->
//    <Inspected>1</Inspected>
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Autoreject>0</Autoreject>
//
//    <!-- ȱ�ݴ��� | ������ | ʵ���޷ϸ��� | ��������-->
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


// �����Ǵ� DataCenter.dll ������
/**
    �������ڽ������XML���ݱ��浽����Ŀ¼��./Data/Satistics.db3��
    ��ṹ������ο��ĵ� �������ݿ��ṹ.xlsx
*/

#include "datacenter_global.h"
#include "ConfigDef.hpp"

#include <QObject>

class DATACENTER_EXPORT DataCenter: public QObject
{
    Q_OBJECT
public:
    DataCenter();
    ~DataCenter();

signals:
    void GetDataCenterConf(DataCenterConf& cfg);
    void SetGenerateRecordTimeInterval(int ti);

public slots:
    /**
    * @brief  :  Init ��ʼ���� �ڲ��������ݿ��Ƿ񴴽������û�д�����ᴴ�����ݿ�
    *
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init(QString* err = nullptr);

    void Stop();

    /**
    * @brief  :  PaserDataToDataBase �������XML��ʽ���ַ������������ݱ��浽���ݿ���
    *
    * @param  :  const QString & xmldata XML��ʽ���ַ������ַ�����ʽ���ļ�ͷע�Ͳ���
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool PaserDataToDataBase(const QString& xmldata, QString* err = nullptr);

    /**
    * @brief  :  signal_GetDataByTime ��ȡ����һ����¼
    *
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool GetLastestRecord(int type, Record& data, QString* err);

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
    bool GetRecordListByDay(int type, QDate date, QList<QTime>& stlst, QList<QTime>& etlst);
     
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

private:
    class Impl;

    Impl* m_pimpl;

};

#endif//DATACANTER_20GCLXZDB2VN57N8_H_

