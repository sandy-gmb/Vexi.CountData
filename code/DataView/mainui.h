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

    void closed();
public:
    setting* m_settingui;

    QMap<int, QString> m_moldwors;        //ģ��ID����
    QMap<int, QString> m_sensorwors;      //ȱ��ID����
private:
    Ui::MainUI *ui;
    QTimer* m_timer;

    Record record;
    bool updated;

};

#endif // MAINUI_H
