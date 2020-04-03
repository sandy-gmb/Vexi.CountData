#ifndef QueryWidget_H
#define QueryWidget_H

#include <QWidget>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QList>
#include <QCloseEvent>

#include "DataDef.hpp"
    
namespace Ui {
class query;
}
class DataView;

class QueryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QueryWidget(DataView* parent);
    ~QueryWidget();

    void Init();

    public slots:
        void on_dateEdit_dateChanged(QDate d);
        void on_listWidget_currentRowChanged(int row);

		void on_rbtn_show_timeinterval_clicked();
		void on_rbtn_show_shift_clicked();

		void on_btn_refresh_clicked();
		void on_btn_to_setting_clicked();
		void on_btn_to_main_clicked();

       // void closeEvent(QCloseEvent *event);

		void ChangeLanguage(const QMap<int, QString>& moldwors, const QMap<int, QString>& sensorwors);

protected:
	void changeEvent(QEvent* event);
private:
	DataView* m_pthis;
    Ui::query *ui;

    QList<QDate> datelst;		//数据库存在的日期类型
    QDate curDate;				//当前显示的日期
    QList<QTime> stlst;			//当前日期的时间段起始时间列表
	QList<QTime> etlst;			//当前日期的时间段结束时间列表
	ERecordType recordtype;		//当前显示记录类型

    QMap<int, QString> m_moldwors;        //模板ID词条
    QMap<int, QString> m_sensorwors;      //缺陷ID词条
};

#endif // QueryWidget_H
