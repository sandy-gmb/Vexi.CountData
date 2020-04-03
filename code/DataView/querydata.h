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

    QList<QDate> datelst;		//���ݿ���ڵ���������
    QDate curDate;				//��ǰ��ʾ������
    QList<QTime> stlst;			//��ǰ���ڵ�ʱ�����ʼʱ���б�
	QList<QTime> etlst;			//��ǰ���ڵ�ʱ��ν���ʱ���б�
	ERecordType recordtype;		//��ǰ��ʾ��¼����

    QMap<int, QString> m_moldwors;        //ģ��ID����
    QMap<int, QString> m_sensorwors;      //ȱ��ID����
};

#endif // QueryWidget_H
