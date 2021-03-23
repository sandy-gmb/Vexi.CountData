#include "setting.h"
#include "ui_setting.h"

#include "UIDataDef.hpp"
#include "dataview.h"
#include "DataDef.hpp"
#include "ConfigDef.hpp"

SettingWidget::SettingWidget(DataView* parent) :
QWidget(nullptr),
    ui(new Ui::setting)
{
    ui->setupUi(this);
    m_pthis = parent;

    setWindowFlags(Qt::FramelessWindowHint);

}

SettingWidget::~SettingWidget()
{
    delete ui;
}

void SettingWidget::Init()
{
    on_btn_reload_clicked();
}

void SettingWidget::on_btn_reload_clicked()
{
    AllConfig _cfg;
    m_pthis->signal_GetAllConfig(_cfg);
    cfg = _cfg;
    //刷新策略信息
    ui->cbtn_mode->setChecked(_cfg.strategy.bStrategyMode);
    ui->lwgt_sensor_id->clear();
    foreach(auto id, _cfg.strategy.lSensorIDs)
    {
        ui->lwgt_sensor_id->addItem(id);
    }
    //刷新系统配置
    ui->cbx_language->setCurrentIndex(_cfg.syscfg.iLanguage);
    ui->ledt_generate_time_interval->setText(QString::number(_cfg.syscfg.iTimeInterval_GeneRecord));
    ui->ledt_get_source_time_interval->setText(QString::number(_cfg.syscfg.iTimeInterval_GetSrcData));
    ui->ledt_outdate_days->setText(QString::number(_cfg.syscfg.iDaysDataOutDate));
    if(_cfg.syscfg.iDefaultShow == ERT_TimeInterval)
    {
        ui->rbtn_show_timeinterval->setChecked(true);
    }
    else
    {
        ui->rbtn_show_shift->setChecked(true);
    }

    switch(_cfg.eTimeInterval)
    {
    case ETI_60_Min:
        ui->rbtn_eti_60min->setChecked(true);
        break;
    case ETI_90_Min:
        ui->rbtn_eti_90min->setChecked(true);
        break;
    case ETI_120_Min:
        ui->rbtn_eti_120min->setChecked(true);
        break;
    case ETI_30_Min:
    default:
        ui->rbtn_eti_30min->setChecked(true);
        break;
    }
    switch (_cfg.syscfg.iProtocolVersion)
    {
    case EPV_VEXI:
        ui->rbtn_protocol_vexi->setChecked(true);
        break;
    case EPV_TERA:
    default:
        ui->rbtn_protocol_tera->setChecked(true);
        break;
    }

    ui->lwgt_shift_time->clear();
    ui->lwgt_shift_time->addItems(_cfg.shiftlst);

}

void SettingWidget::on_btn_to_query_clicked()
{
    m_pthis->OnChangeUI(EUI_QueryData);
}

void SettingWidget::on_btn_to_main_clicked()
{
    m_pthis->OnChangeUI(EUI_Main);
}

void SettingWidget::on_btn_save_clicked()
{
    AllConfig _cfg;
    _cfg.strategy = cfg.strategy;
    _cfg.syscfg.sCodetsprefix = cfg.syscfg.sCodetsprefix;
    _cfg.dbgcfg = cfg.dbgcfg;

    _cfg.syscfg.iLanguage = ui->cbx_language->currentIndex();
    _cfg.syscfg.iTimeInterval_GeneRecord = ui->ledt_generate_time_interval->text().toInt();
    _cfg.syscfg.iTimeInterval_GetSrcData = ui->ledt_get_source_time_interval->text().toInt();
    _cfg.syscfg.iDaysDataOutDate = ui->ledt_outdate_days->text().toInt();
    if(ui->rbtn_show_shift->isChecked())
    {
        _cfg.syscfg.iDefaultShow = 1;
    }
    else
    {
        _cfg.syscfg.iDefaultShow = 0;
    }

    if (ui->rbtn_protocol_vexi->isChecked())
    {
        _cfg.syscfg.iProtocolVersion = EPV_VEXI;
    }
    else
    {
        _cfg.syscfg.iProtocolVersion = EPV_TERA;
    }

    if(ui->rbtn_eti_60min->isChecked())
    {

        _cfg.eTimeInterval = ETI_60_Min;
    }
    else if(ui->rbtn_eti_90min->isChecked())
    {

        _cfg.eTimeInterval = ETI_90_Min;
    }
    else if(ui->rbtn_eti_120min->isChecked())
    {

        _cfg.eTimeInterval = ETI_120_Min;
    }
    else 
    {
        _cfg.eTimeInterval = ETI_30_Min;
    }


    int rowcnt = ui->lwgt_shift_time->count();
    for(int i = 0; i < rowcnt; i++)
    {
        _cfg.shiftlst.append(ui->lwgt_shift_time->item(i)->text());
    }

    if(cfg != _cfg)
    {
        m_pthis->signal_SetAllConfig(_cfg);
        cfg = _cfg;
    }
}

void SettingWidget::on_btn_add_shifttime_clicked()
{
    QString t = ui->tedt_edit->text();
    if(ui->lwgt_shift_time->findItems(t, Qt::MatchFixedString).isEmpty())
    {
        ui->lwgt_shift_time->addItem(t);
    }
}

void SettingWidget::on_btn_delete_shifttime_clicked()
{
    int i = ui->lwgt_shift_time->currentRow();
    if(i >= 0)
    {
        QListWidgetItem* t = ui->lwgt_shift_time->takeItem(i);
        delete t;
        if(ui->lwgt_shift_time->count() >= i)
        {
            ui->lwgt_shift_time->setCurrentRow(i);
        }
        else if(ui->lwgt_shift_time->count() > 0)
        {
            ui->lwgt_shift_time->setCurrentRow(i-1);
        }
    }
}

void SettingWidget::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    switch (event->type()) 
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            on_btn_reload_clicked();
            break;
        default:
            break;
    }
}
