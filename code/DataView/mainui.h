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

    void Init(ERecordType type);
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
	ERecordType recordtype;		//当前显示记录类型
	ETimeInterval curETI;		//当前时间间隔 根据配置刷新
	//班次信息由于跟时间相关,并且已在记录中 不需要临时变量

	QMap<int, QString> m_moldwors;        //模板ID词条
	QMap<int, QString> m_sensorwors;      //缺陷ID词条
	bool isinit;
};

#endif // MAINUI_H
