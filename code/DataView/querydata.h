#ifndef QueryWidget_H
#define QueryWidget_H

#include <QWidget>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QList>
#include <QCloseEvent>

#include "DataDef.hpp"
#include "ConfigDef.hpp"
#include "UIDataDef.hpp"
    
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

        void updateData(const SimpleRecord& r);

        void on_rbtn_show_timeinterval_clicked();
        void on_rbtn_show_shift_clicked();

        void on_btn_refresh_clicked();
        void on_btn_to_setting_clicked();
        void on_btn_to_main_clicked();

       // void closeEvent(QCloseEvent *event);

        void ChangeLanguage(const QMap<QString, QString>& moldwors, const QMap<QString, QString>& sensorwors);

protected:
    void changeEvent(QEvent* event);
private:
    DataView* m_pthis;
    Ui::query *ui;

    QList<QDate> datelst;           //���ݿ���ڵ���������
    QDate curDate;                  //��ǰ��ʾ������
    QList<QDateTime> stlst;         //��ǰ���ڵ�ʱ�����ʼʱ���б�
    QList<QDateTime> etlst;         //��ǰ���ڵ�ʱ��ν���ʱ���б�
    ERecordType recordtype;         //��ǰ��ʾ��¼����

    QMap<QString, QString> m_moldwors;        //ģ��ID����
    QMap<QString, QString> m_sensorwors;      //ȱ��ID����

    SystemConfig syscfg;
};

#endif // QueryWidget_H
