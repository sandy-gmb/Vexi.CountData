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
class DataView;

class SettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingWidget(DataView* parent);
    ~SettingWidget();

    void Init();

public slots:
		void ChangeLanguage();

		void on_btn_reload_clicked();
		void on_btn_to_query_clicked();
		void on_btn_to_main_clicked();
		void on_btn_save_clicked();
		void on_btn_add_shifttime_clicked();
		void on_delete_shifttime_clicked();
private:
	DataView* m_pthis;
    Ui::setting *ui;

    AllConfig cfg;
};

#endif // SETTING_H
