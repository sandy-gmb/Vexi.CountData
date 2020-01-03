#ifndef MAINUI_H
#define MAINUI_H

#include <QWidget>

#include "DataDef.hpp"

namespace Ui {
class MainUI;
}

class QTimer;
class setting;

class MainUI : public QWidget
{
    Q_OBJECT

public:
    explicit MainUI(QWidget *parent = 0);
    ~MainUI();

    void Init();
    public slots:
        void GetData();
        void updateUI();

        void on_btn_settings_clicked();

        void UpdateTimeInterval(ETimeInterval ti);

        void closeEvent(QCloseEvent *event);
signals:
   /**
    * @brief  :  signal_GetDataByTime 获取最新一条记录
    *
    * @param  :  Record & data 返回数据
    * @return :  bool 可能由于异常原因导致出错(如数据库文件无法打开等)
    * @retval :
    */
    bool signal_GetLastestRecord(Record& data, QString* err);

    /**
    * @brief  :  GetTimeInterval 获取时间间隔
    *
    * @return :  ETimeInterval
    * @retval :
    */
    ETimeInterval signal_GetTimeInterval();

    void closed();
public:
    setting* m_settingui;

    QMap<int, QString> m_moldwors;        //模板ID词条
    QMap<int, QString> m_sensorwors;      //缺陷ID词条
private:
    Ui::MainUI *ui;
    QTimer* m_timer;

    Record record;
    bool updated;

};

#endif // MAINUI_H
