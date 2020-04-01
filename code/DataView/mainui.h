#ifndef MAINUI_H
#define MAINUI_H

#include <QWidget>

#include "DataDef.hpp"

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

    void Init();
    public slots:
        void GetData();
        void updateUI();

        void UpdateTimeInterval(ETimeInterval ti);

        void closeEvent(QCloseEvent *event);

		void ChangeLanguage(const QMap<int, QString>& moldwors, const QMap<int, QString>& sensorwors);
		void ChangedShowRecordT(ERecordType type);
		void refreshWidget();

		void on_btn_to_query_clicked();
		void on_btn_to_settings_clicked();
signals:
    void closed();

private:
	DataView* m_pthis;
    Ui::MainUI *ui;

    QTimer* m_timer;

    Record record;
	bool updated;
	ERecordType recordtype;		//��ǰ��ʾ��¼����
	ETimeInterval curETI;		//��ǰʱ���� ��������ˢ��
	//�����Ϣ���ڸ�ʱ�����,�������ڼ�¼�� ����Ҫ��ʱ����

	QMap<int, QString> m_moldwors;        //ģ��ID����
	QMap<int, QString> m_sensorwors;      //ȱ��ID����

};

#endif // MAINUI_H
