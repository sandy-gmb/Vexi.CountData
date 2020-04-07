#include "querydata.h"
#include "ui_querydata.h"

#include "UIDataDef.hpp"
#include "mainui.h"
#include "dataview.h"

QueryWidget::QueryWidget(DataView* parent) :
    QWidget(nullptr),
    ui(new Ui::query)
{
    m_pthis = parent;
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    
}

QueryWidget::~QueryWidget()
{
    delete ui;
}

void QueryWidget::on_dateEdit_dateChanged( QDate d )
{
    curDate = d;
    m_pthis->signal_GetRecordListByDay(recordtype, d, stlst, etlst );
    ui->listWidget->clear();
    QStringList l;
    for (int i = 0; i < stlst.size();i++)
    {
        l.append(QString("%1:%2_%3:%4").arg(stlst[i].time().hour(),2, 10, QChar('0')).arg(stlst[i].time().minute(),2, 10, QChar('0'))
            .arg( etlst[i].time().hour(), 2, 10, QChar('0')).arg(etlst[i].time().minute(), 2, 10, QChar('0')));
    }
	ui->listWidget->addItems(l);
	if(!l.isEmpty())
	{
		ui->listWidget->setCurrentRow(0);
	}
}   

void QueryWidget::on_listWidget_currentRowChanged( int row )
{
    if(row == -1)
        return;
    QDateTime st = stlst[row];
    QDateTime et = etlst[row];
    Record record;
    m_pthis->signal_GetRecordByTime(recordtype, st, et, record);

    SimpleRecord r;
    r.SetRecord(record);

    ui->tb_record->clear();
    if(r.inspected == 0)
    {
        ui->tb_record->setRowCount(0);
        ui->tb_record->setColumnCount(0);
        ui->ledt_starttime->setText("");
		ui->ledt_endtime->setText("");
		ui->l_error->setText(tr("No Data"));
		ui->l_error->setStyleSheet("color: rgb(255, 0, 0);font: 11pt \"Times New Roman\";");
    }
    else
    {
		if(recordtype == ERT_Shift)
		{
			ui->l_error->setText(tr("Shift:%1-%2").arg(r.shiftDate.toString("yyyy.MM.dd")).arg(r.shift));
			ui->l_error->setStyleSheet("color: rgb(0, 0, 0);font: 11pt \"Times New Roman\";");
		}
		else
		{
			ui->l_error->setText("");
		}

        ui->ledt_starttime->setText(r.dt_start.time().toString("hh:mm"));
        ui->ledt_endtime->setText(r.dt_end.time().toString("hh:mm"));
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
		ui->tb_record->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents); 
    }
}

void QueryWidget::on_rbtn_show_timeinterval_clicked()
{
	recordtype = ERT_TimeInterval;
	on_btn_refresh_clicked();
}

void QueryWidget::on_rbtn_show_shift_clicked()
{
	recordtype = ERT_Shift;
	on_btn_refresh_clicked();
}

void QueryWidget::on_btn_refresh_clicked()
{
    //获取有记录的日期列表,并保存
    m_pthis->signal_GetAllDate(recordtype, datelst);
    if(!datelst.isEmpty())
    {
        //设置最新日期为默认日期
        ui->dateEdit->setMinimumDate(datelst.first());
		ui->dateEdit->setMaximumDate(datelst.last());
		ui->l_error->setText("");
        curDate = datelst.first();
		ui->dateEdit->setDate(curDate);
		//ui->listWidget->setCurrentRow(0);
    }
    else
    {
        curDate = QDate::currentDate();
        ui->dateEdit->setMinimumDate(curDate);
        ui->dateEdit->setMaximumDate(curDate);
		ui->dateEdit->setDate(curDate);
		ui->l_error->setText(tr("No Latest Data"));
		ui->l_error->setStyleSheet("color: rgb(255, 0, 0);font: 11pt \"Times New Roman\";");
    }
}

void QueryWidget::on_btn_to_setting_clicked()
{
	m_pthis->OnChangeUI(EUI_Settings);
}

void QueryWidget::Init()
{
	recordtype = ERT_TimeInterval;
	ui->rbtn_show_timeinterval->setChecked(true);
}

void QueryWidget::on_btn_to_main_clicked()
{
	m_pthis->OnChangeUI(EUI_Main);
}

void QueryWidget::ChangeLanguage(const QMap<int, QString>& moldwors, const QMap<int, QString>& sensorwors)
{
	m_moldwors = moldwors;
	m_sensorwors = sensorwors;
}

void QueryWidget::changeEvent(QEvent* event)
{
	QWidget::changeEvent(event);
	switch (event->type()) 
	{
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
