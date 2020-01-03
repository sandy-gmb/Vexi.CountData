#ifndef SETTING_H
#define SETTING_H

#include <QWidget>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QList>
#include <QCloseEvent>

#include "DataDef.hpp"
    
namespace Ui {
class setting;
}

class setting : public QWidget
{
    Q_OBJECT

public:
    explicit setting(QWidget *main, QWidget *parent = 0);
    ~setting();

    void Init();

    public slots:
        void on_dateEdit_dateChanged(QDate d);
        void on_listWidget_currentRowChanged(int row);

        void on_rbtn_eti_30min_clicked();
        void on_rbtn_eti_60min_clicked();
        void on_rbtn_eti_90min_clicked();
        void on_rbtn_eti_120min_clicked();

        void on_btn_refresh_clicked();
        void on_btn_goback_clicked();

       // void closeEvent(QCloseEvent *event);

signals:
    bool signal_GetAllDate(QList<QDate>& lst);

    bool signal_GetRecordListByDay(QDate date, QList<QTime>& stlst, QList<QTime>& etlst);

    bool signal_GetRecordByTime(QDateTime st, QDateTime et, Record& r);

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

public:
    QMap<int, QString> m_moldwors;        //ģ��ID����
    QMap<int, QString> m_sensorwors;      //ȱ��ID����
private:
    Ui::setting *ui;

    QList<QDate> datelst;
    QDate curDate;
    QList<QTime> stlst;
    QList<QTime> etlst;

    QWidget* mainui;

};

#endif // SETTING_H
