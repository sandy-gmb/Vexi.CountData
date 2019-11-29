/********************************************************************
* @filename  : dataview.h
*
* @brief     : ���ļ����ڽ�������ʾ��������
*	
* @author    : Guo.Mingbing
* @date      : 2019/11/25
* 
* @version   : V0.1
*********************************************************************/
#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "dataview_global.h"

#include <QObject>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QList>

#include "DataDef.hpp"

class DATAVIEW_EXPORT DataView:public QObject
{
    Q_OBJECT
public:
    DataView(QObject* parent = nullptr);
    ~DataView();

public slots:
    void show();
signals:
    /**
    * @brief  :  signal_GetDataByTime ��ȡ����һ����¼
    *
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool signal_GetLastestRecord(Record& data, QString* err);

    /**
    * @brief  :  GetTimeInterval ��ȡʱ����
    *
    * @return :  ETimeInterval
    * @retval :
    */
    ETimeInterval signal_GetTimeInterval();

    /**
    * @brief  :  SetTimeInterval
    *
    * @param  :  ETimeInterval timeinterval
    * @return :  void
    * @retval :
    */
    void signal_SetTimeInterval(ETimeInterval timeinterval);

    /**
    * @brief  :  signal_GetAllDate ��ȡ��ǰ��¼�������б�
    *
    * @param  :  QList<QDate> & lst
    * @return :  bool
    * @retval :
    */
    bool signal_GetAllDate(QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay ��ȡָ�����ڵļ�¼��ʼʱ��ͽ���ʱ���б�
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    bool signal_GetRecordListByDay(QDate date, QList<QTime>& stlst, QList<QTime>& etlst);
     
    /**
    * @brief  :  signal_GetDataByTime ��ʱ���ȡ����,�൱�����ÿ�����������Դ,�������ӵĲ�ͬ��Դ�Ĳۺ���
    *
    * @param  :  QDateTime st   ��ʼʱ��
    * @param  :  QDateTime end  ����ʱ��
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool signal_GetRecordByTime(QDateTime st, QDateTime end, Record& data);

    void closed();

private:
    class Impl;
    
    Impl* pimpl;
};

#endif // DATAVIEW_H
