/********************************************************************
* @filename  : DataCenter.h
*
* @brief     : ��ģ�����ڽ���ϵͳ��webservice���ص�XML�ַ����������浽���ݿ���
*	
* @author    : Mingbing.Guo
* @date      : 2019-11-12
* 
* @version   : V0.1


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
    void GetSystemConf(SystemConfig& cfg);
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
    bool GetRecordListByDay(int type, QDate date, QList<QDateTime>& stlst, QList<QDateTime>& etlst);
     
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

	//void RecordConfigChanged();
	//void OnDataConfChange(const DataCenterConf& cfg);

private:
    class Impl;

    Impl* m_pimpl;

};

#endif//DATACANTER_20GCLXZDB2VN57N8_H_

