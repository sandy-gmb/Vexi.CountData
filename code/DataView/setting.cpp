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

void setting::on_dateEdit_dateChanged( QDate d )
{
    curDate = d;
    signal_GetRecordListByDay(d, stlst, etlst );
    ui->listWidget->clear();
    QStringList l;
    for (int i = 0; i < stlst.size();i++)
    {
        l.append(QString("%1:%2_%3:%4").arg(stlst[i].hour(),2, 10, QChar('0')).arg(stlst[i].minute(),2, 10, QChar('0'))
            .arg( etlst[i].hour(), 2, 10, QChar('0')).arg(etlst[i].minute(), 2, 10, QChar('0')));
    }
    ui->listWidget->insertItems(0, l);
    if(l.size() != 0)
    {
        ui->listWidget->setCurrentRow(l.size()-1);
    }
    else{
        ui->tb_record->setColumnCount(0);
        ui->tb_record->setRowCount(0);
    }
}   

void setting::on_listWidget_currentRowChanged( int row )
{
    if(row == -1)
        return;
    QDateTime st(curDate, stlst[row]);
    QDateTime et(curDate, etlst[row]);
    Record record;
    signal_GetRecordByTime(st, et, record);

    SimpleRecord r;
    r.SetRecord(record);

    ui->tb_record->clear();
    if(r.inspected == 0)
    {
        ui->tb_record->setRowCount(0);
        ui->tb_record->setColumnCount(0);
        ui->ledt_starttime->setText("");
        ui->ledt_endtime->setText("");
    }
    else
    {
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
            QString colheader = tr("%1").arg(p.first);
            if(m_moldwors.contains(p.first))
            {
                colheader = m_moldwors[p.first];
            }
            //添加列表头
            ui->tb_record->setHorizontalHeaderItem(i+2, new QTableWidgetItem( colheader));
            coln_idx.insert(p.first, i+2);
            //添加数据
            ui->tb_record->setItem(r.sensor_rejects.size()+2-2, i+2, new QTableWidgetItem(tr("%1%").arg(double(p.second/(r.rejects*1.0)*100), 0, 'f', 1)));
            ui->tb_record->setItem(r.sensor_rejects.size()+2-1, i+2, new QTableWidgetItem(tr("%1").arg(p.second)));
        }

        //设置缺陷行数据 添加每个缺陷的行头,错误率和错误总数
        for(int i = 0; i < r.sensor_rejects.size();i++)
        {
            QPair<int, int>& p = r.sensor_rejects[i];
            QString rowheader = tr("%1").arg(p.first);
            if(m_sensorwors.contains(p.first))
            {
                rowheader = m_sensorwors[p.first];
            }
            //添加行表头
            ui->tb_record->setVerticalHeaderItem(i, new QTableWidgetItem( rowheader));
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

void setting::on_btn_refresh_clicked()
{
    //获取有记录的日期列表,并保存
    signal_GetAllDate(datelst);
    if(!datelst.isEmpty())
    {
        //设置最新日期为默认日期
        ui->dateEdit->setMinimumDate(datelst.first());
        ui->dateEdit->setMaximumDate(datelst.last());
        curDate = datelst.last();
		on_dateEdit_dateChanged(datelst.last());
    }
    else
    {
        curDate = QDate::currentDate();
        ui->dateEdit->setMinimumDate(curDate);
        ui->dateEdit->setMaximumDate(curDate);
        ui->dateEdit->setDate(curDate);
    }
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
    ui->listWidget->clear();
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
