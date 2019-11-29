#include "mainui.h"
#include "ui_mainui.h"

#include <QMap>
#include <QPair>
#include <QList>
#include <QtAlgorithms>
#include <QTimer>

#include "setting.h"
#include "UIDataDef.hpp"

MainUI::MainUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainUI)
{
    ui->setupUi(this);
    updated = false;
}

MainUI::~MainUI()
{
    m_timer->stop();
    
    delete ui;
}

void MainUI::GetData()
{
    bool rt = false;
    rt = signal_GetLastestRecord(record, nullptr);
    if(rt )
    {
       updated = true; 
    }
}

void MainUI::updateUI()
{
    if(!updated)
    {
        return;
    }
    SimpleRecord r;
    r.SetRecord(record);
    updated = false;

    ui->tb_record->clear();
    if(r.inspected == 0)
    {
        ui->l_error->setText(tr("No Lastest Data"));
        ui->ledt_date->setText("");
        ui->ledt_starttime->setText("");
        ui->ledt_endtime->setText("");
    }
    else
    {
        ui->l_error->setText("");
        ui->ledt_date->setText(r.dt_start.date().toString("yyyy-MM-dd"));
        ui->ledt_starttime->setText(r.dt_start.time().toString("hh:mm:ss"));
        ui->ledt_endtime->setText(r.dt_end.time().toString("hh:mm:ss"));
        ui->ledt_total->setText(QString::number(r.inspected));
        ui->ledt_reject->setText(QString::number(r.rejects));
        ui->ledt_rate->setText(QString("%1%").arg(r.rejects/(r.inspected*1.0)*100, 0,'f', 1));
        
        //模板个数+合计+错误占比
        QMap<int, int> coln_idx, roln_idx; // 模板对应列号 缺陷对应行号
        ui->tb_record->setColumnCount(r.mold_rejects.size()+2);
        ui->tb_record->setRowCount(r.sensor_rejects.size()+2);
        ui->tb_record->setHorizontalHeaderItem(0, new QTableWidgetItem( tr("Total")));
        ui->tb_record->setHorizontalHeaderItem(1, new QTableWidgetItem( tr("ErrorRate")));

        ui->tb_record->setVerticalHeaderItem(r.sensor_rejects.size()+2-2, new QTableWidgetItem( tr("ErrorRate")));
        ui->tb_record->setVerticalHeaderItem(r.sensor_rejects.size()+2-1, new QTableWidgetItem( tr("Total")));
        //填写合计和错误占比信息
        ui->tb_record->setItem(r.sensor_rejects.size()+2-2, 0, new QTableWidgetItem(tr("-")));
        ui->tb_record->setItem(r.sensor_rejects.size()+2-2, 1, new QTableWidgetItem(tr("-")));
        ui->tb_record->setItem(r.sensor_rejects.size()+2-1, 0, new QTableWidgetItem(tr("%1").arg(r.rejects)));
        ui->tb_record->setItem(r.sensor_rejects.size()+2-1, 1, new QTableWidgetItem(tr("-")));

        //添加模板 列头和每个模板的错误率和错误总数
        for(int i = 0; i < r.mold_rejects.size();i++)
        {
            QPair<int, int>& p = r.mold_rejects[i];
            //添加列表头
            ui->tb_record->setHorizontalHeaderItem(i+2, new QTableWidgetItem( tr("%1").arg(p.first)));
            coln_idx.insert(p.first, i+2);
            //添加数据
            ui->tb_record->setItem(r.sensor_rejects.size()+2-2, i+2, new QTableWidgetItem(tr("%1%").arg(double(p.second/(r.rejects*1.0)*100), 0, 'f', 1)));
            ui->tb_record->setItem(r.sensor_rejects.size()+2-1, i+2, new QTableWidgetItem(tr("%1").arg(p.second)));
        }

        //设置缺陷行数据 添加每个缺陷的行头,错误率和错误总数
        for(int i = 0; i < r.sensor_rejects.size();i++)
        {
            QPair<int, int>& p = r.sensor_rejects[i];
            //添加列表头
            ui->tb_record->setVerticalHeaderItem(i, new QTableWidgetItem( tr("%1").arg(p.first)));
            roln_idx.insert(p.first, i);
            //添加数据
            ui->tb_record->setItem(i, 1, new QTableWidgetItem(tr("%1%").arg(double(p.second/(r.rejects*1.0)*100), 0, 'f', 1)));
            ui->tb_record->setItem(i, 0, new QTableWidgetItem(tr("%1").arg(p.second)));
        }
        //添加剩余数据
        foreach(int mid, r.mold_sensor_rejects.keys())
        {//竖直添加
            QMap<int, int>& sensor_rej = r.mold_sensor_rejects[mid];
            foreach(int sid, sensor_rej.keys())
            {
                ui->tb_record->setItem( roln_idx[sid], coln_idx[mid], new QTableWidgetItem(QString::number(sensor_rej[sid])));
            }
        }
    }
}

void MainUI::on_btn_settings_clicked()
{
    hide();
    m_settingui->on_btn_refresh_clicked();
    m_settingui->show();
}

void MainUI::UpdateTimeInterval( ETimeInterval ti )
{
    int eti = ETimeInterval2Min(ti);
    ui->l_timeinterval->setText(tr("Current Time Interval:%1 Min").arg(eti));
}

void MainUI::Init()
{
    ETimeInterval ti = signal_GetTimeInterval();

    UpdateTimeInterval(ti);
    m_timer = new QTimer(this);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(GetData()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateUI()));

    m_timer->start(2000);
}

void MainUI::closeEvent( QCloseEvent *event )
{
    m_timer->stop();

    emit closed();
    m_settingui->close();
    event->accept();
}
