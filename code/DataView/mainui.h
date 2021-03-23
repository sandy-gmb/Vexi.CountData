#ifndef MAINUI_H
#define MAINUI_H

#include <QWidget>

#include "DataDef.hpp"
#include "ConfigDef.hpp"
#include "UIDataDef.hpp"

namespace Ui {
class MainUI;
}

class QTimer;
class DataView;

class MainUI : public QWidget
{
    Q_OBJECT

public:
    explicit MainUI(DataView* parent);
    ~MainUI();

    void Init(ERecordType type, QString title = "");
    public slots:
        void GetData();
        void updateUI();
        void updateData(const SimpleRecord& r);

        void UpdateTimeInterval(ETimeInterval ti);

        void closeEvent(QCloseEvent *event);

        void ChangeLanguage(const QMap<QString, QString>& moldwors, const QMap<QString, QString>& sensorwors);
        void ChangedShowRecordT(ERecordType type);
        void refreshWidget();

        void on_btn_to_query_clicked();
        void on_btn_to_settings_clicked();

protected:
        void changeEvent(QEvent* event);
signals:
    void closed();

private:
    DataView* m_pthis;
    Ui::MainUI *ui;

    QTimer* m_timer;

    Record record;
    bool updated;
    ERecordType recordtype;        //��ǰ��ʾ��¼����
    ETimeInterval curETI;        //��ǰʱ���� ��������ˢ��
    //�����Ϣ���ڸ�ʱ�����,�������ڼ�¼�� ����Ҫ��ʱ����

    QMap<QString, QString> m_moldwors;        //ģ��ID����
    QMap<QString, QString> m_sensorwors;      //ȱ��ID����
    bool isinit;

    QString strTitle;

    SystemConfig syscfg;
};

#endif // MAINUI_H
