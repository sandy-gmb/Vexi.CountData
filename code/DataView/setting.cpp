#include "setting.h"
#include "ui_setting.h"

#include "UIDataDef.hpp"
#include "mainui.h"

setting::setting(QWidget *main, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::setting)
{
    mainui = main;
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    
}

setting::~setting()
{
    delete ui;
}

void setting::on_rbtn_eti_60min_clicked()
{
    signal_SetTimeInterval(ETI_60_Min);
    if(mainui!= nullptr)
    {
        ((MainUI*)mainui)->UpdateTimeInterval(ETI_60_Min);
    }
}

void setting::on_rbtn_eti_30min_clicked()
{
    signal_SetTimeInterval(ETI_30_Min);
    if(mainui!= nullptr)
    {
        ((MainUI*)mainui)->UpdateTimeInterval(ETI_30_Min);
    }
}

void setting::on_rbtn_eti_90min_clicked()
{
    signal_SetTimeInterval(ETI_90_Min);
    if(mainui!= nullptr)
    {
        ((MainUI*)mainui)->UpdateTimeInterval(ETI_90_Min);
    }
}

void setting::on_rbtn_eti_120min_clicked()
{
    signal_SetTimeInterval(ETI_120_Min);
    if(mainui!= nullptr)
    {
        ((MainUI*)mainui)->UpdateTimeInterval(ETI_120_Min);
    }
}

//void setting::closeEvent( QCloseEvent *event )
//{
//    mainui->close();
//    event->accept();
//}

void setting::Init()
{
    //根据时间间隔 设置界面显示的时间间隔
    ETimeInterval ti = signal_GetTimeInterval();
    switch(ti)
    {
    case ETI_30_Min:
        ui->rbtn_eti_30min->setChecked(true);
        break;
    case ETI_60_Min:
        ui->rbtn_eti_60min->setChecked(true);
        break;
    case ETI_90_Min:
        ui->rbtn_eti_90min->setChecked(true);
        break;
    case ETI_120_Min:
        ui->rbtn_eti_120min->setChecked(true);
        break;
    }

    on_btn_refresh_clicked();
}

void setting::on_btn_goback_clicked()
{
    this->hide(); 
    mainui->show();
}
