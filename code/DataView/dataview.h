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
#include "ConfigDef.hpp"

class DATAVIEW_EXPORT DataView:public QObject
{
    Q_OBJECT
public:
    DataView( QObject* parent = nullptr);
    ~DataView();

public slots:
    void Init(QString title = "");
    void show();

    void OnLanguageChange(int lang);
    void OnChangeUI(EUISelection sel);
    void OnRecordShowChanged(int show);
signals:
    /****************  Config **********************************************************/
    void signal_GetAllConfig(AllConfig& cfg);
    void signal_SetAllConfig(const AllConfig& cfg);
    void signal_GetWordsTranslation(QMap<QString, QString>& , QMap<QString, QString>&);
    int signal_GetTimeInterval();


    /****************  DataCenter ******************************************************/
    /**
    * @brief  :  signal_GetDataByTime ��ȡ����һ����¼
    *
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool signal_GetLastestRecord(int type, Record& data, QString* err);

    /**
    * @brief  :  signal_GetAllDate ��ȡ��ǰ��¼�������б�
    *
    * @param  :  QList<QDate> & lst
    * @return :  bool
    * @retval :
    */
    bool signal_GetAllDate(int type, QList<QDate>& lst);

    /**
    * @brief  :  signal_GetRecordListByDay ��ȡָ�����ڵļ�¼��ʼʱ��ͽ���ʱ���б�
    *
    * @param  :  QDate date
    * @param  :  QList<QTime> & stlst
    * @param  :  QList<QTime> & etlst
    * @return :  bool
    * @retval :
    */
    bool signal_GetRecordListByDay(int type, QDate date, QList<QDateTime>& stlst, QList<QDateTime>& etlst);
     
    /**
    * @brief  :  signal_GetDataByTime ��ʱ���ȡ����,�൱�����ÿ�����������Դ,�������ӵĲ�ͬ��Դ�Ĳۺ���
    *
    * @param  :  QDateTime st   ��ʼʱ��
    * @param  :  QDateTime end  ����ʱ��
    * @param  :  Record & data ��������
    * @return :  bool ���������쳣ԭ���³���(�����ݿ��ļ��޷��򿪵�)
    * @retval :
    */
    bool signal_GetRecordByTime(int type, QDateTime st, QDateTime end, Record& data);


    /****************  Self Defined ******************************************************/
    void closed();

private:
    class Impl;
    
    Impl* pimpl;

    friend class MainUI;
    friend class QueryWidget;
    friend class SettingWidget;

};

#endif // DATAVIEW_H
