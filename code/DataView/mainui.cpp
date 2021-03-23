#include "mainui.h"
#include "ui_mainui.h"
#include "dataview.h"

#include <QMap>
#include <QPair>
#include <QList>
#include <QtAlgorithms>
#include <QTimer>
#include <QHeaderView>

#include "setting.h"
#include "UIDataDef.hpp"

MainUI::MainUI(DataView* parent) :
    QWidget(nullptr),
    ui(new Ui::MainUI),
    isinit(false)
{
    m_pthis = parent;
    ui->setupUi(this);
    AllConfig cfg;
    m_pthis->signal_GetAllConfig(cfg);
    syscfg = cfg.syscfg;
}

MainUI::~MainUI()
{
    m_timer->stop();

    delete ui;
}

void MainUI::GetData()
{
    if (!this->isVisible())
    {
        //return;
    }
    bool rt = false;
    Record r;
    rt = m_pthis->signal_GetLastestRecord((int)recordtype, r, nullptr);
    if (rt)
    {
        if (record.dt_end > r.dt_end && record.dt_start > r.dt_start)
            return;
        record = r;
        updated = true;
    }
}

void MainUI::updateUI()
{
    if (!updated)
    {
        return;
    }

    updated = false;

    ui->tb_record->clear();
    if (record.inspected == 0)
    {
        ui->tb_record->setRowCount(0);
        ui->tb_record->setColumnCount(0);
        ui->l_error->setText(tr("No Latest Data"));
        ui->ledt_date->setText("");
        ui->ledt_starttime->setText("");
        ui->ledt_endtime->setText("");
    }
    else
    {
        SimpleRecord r;
        switch (syscfg.iProtocolVersion)
        {
        case EPV_VEXI:
            r.SetRecord(record);
            break;
        case EPV_TERA:
        default:
            r.SetRecordV2(record);
            break;
        }
        updateData(r);
    }
}

void MainUI::updateData(const SimpleRecord& r)
{
    if (recordtype == ERT_Shift)
    {
        ui->l_timeinterval->setText(tr("Shift:%1-%2.").arg(r.shiftDate.toString("yyyy.MM.dd")).arg(r.shift + 1));
    }
    ui->l_error->setText("");
    ui->ledt_date->setText(r.dt_start.date().toString("yyyy-MM-dd"));
    ui->ledt_starttime->setText(r.dt_start.time().toString("hh:mm"));
    ui->ledt_endtime->setText(r.dt_end.time().toString("hh:mm"));
    ui->ledt_total->setText(QString::number(r.inspected));
    ui->ledt_reject->setText(QString::number(r.rejects));
    ui->ledt_rate->setText(QString("%1%").arg(r.rejects / (r.inspected * 1.0) * 100, 0, 'f', 1));

    //模板个数+合计+错误占比
    QMap<QString, int> coln_idx, roln_idx; // 模板对应列号 缺陷对应行号
    ui->tb_record->setColumnCount(r.mold_rejects.size() + 2);
    ui->tb_record->setRowCount(r.sensor_rejects.size() + 2);
    ui->tb_record->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Total")));
    ui->tb_record->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("ErrorRate")));

    ui->tb_record->setVerticalHeaderItem(r.sensor_rejects.size() + 2 - 2, new QTableWidgetItem(tr("ErrorRate")));
    ui->tb_record->setVerticalHeaderItem(r.sensor_rejects.size() + 2 - 1, new QTableWidgetItem(tr("Total")));
    //填写合计和错误占比信息
    ui->tb_record->setItem(r.sensor_rejects.size() + 2 - 2, 0, new QTableWidgetItem(tr("-")));
    ui->tb_record->setItem(r.sensor_rejects.size() + 2 - 2, 1, new QTableWidgetItem(tr("-")));
    ui->tb_record->setItem(r.sensor_rejects.size() + 2 - 1, 0, new QTableWidgetItem(tr("%1").arg(r.rejects)));
    ui->tb_record->setItem(r.sensor_rejects.size() + 2 - 1, 1, new QTableWidgetItem(tr("-")));

    //添加模板 列头和每个模板的错误率和错误总数
    for (int i = 0; i < r.mold_rejects.size(); i++)
    {
        auto& p = r.mold_rejects[i];
        QString colheader = tr("%1").arg(p.first);
        if (m_moldwors.contains(p.first))
        {
            colheader = m_moldwors[p.first];
        }
        //添加列表头
        ui->tb_record->setHorizontalHeaderItem(i + 2, new QTableWidgetItem(colheader));
        coln_idx.insert(p.first, i + 2);
        //添加数据
        ui->tb_record->setItem(r.sensor_rejects.size() + 2 - 2, i + 2, new QTableWidgetItem(tr("%1%").arg(double(p.second / (r.rejects * 1.0) * 100), 0, 'f', 1)));
        ui->tb_record->setItem(r.sensor_rejects.size() + 2 - 1, i + 2, new QTableWidgetItem(tr("%1").arg(p.second)));
    }

    //设置缺陷行数据 添加每个缺陷的行头,错误率和错误总数
    for (int i = 0; i < r.sensor_rejects.size(); i++)
    {
        auto& p = r.sensor_rejects[i];
        QString rowheader = tr("%1").arg(p.first);
        if (m_sensorwors.contains(p.first))
        {
            rowheader = m_sensorwors[p.first];
        }
        //添加行表头
        ui->tb_record->setVerticalHeaderItem(i, new QTableWidgetItem(rowheader));
        roln_idx.insert(p.first, i);
        //添加数据
        ui->tb_record->setItem(i, 1, new QTableWidgetItem(tr("%1%").arg(double(p.second / (r.rejects * 1.0) * 100), 0, 'f', 1)));
        ui->tb_record->setItem(i, 0, new QTableWidgetItem(tr("%1").arg(p.second)));
    }
    //添加剩余数据
    foreach(auto mid, r.mold_sensor_rejects.keys())
    {//竖直添加
        auto& sensor_rej = r.mold_sensor_rejects[mid];
        foreach(auto sid, sensor_rej.keys())
        {
            ui->tb_record->setItem(roln_idx[sid], coln_idx[mid], new QTableWidgetItem(QString::number(sensor_rej[sid])));
        }
    }
    ui->tb_record->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainUI::UpdateTimeInterval(ETimeInterval ti)
{
    int eti = ETimeInterval2Min(ti);
    ui->l_timeinterval->setText(tr("Current Time Interval:%1 Min").arg(eti));
}

void MainUI::Init(ERecordType type, QString title)
{
    strTitle = title;
    recordtype = type;
    m_timer = new QTimer(this);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(GetData()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateUI()));

    m_timer->start(2000);
    isinit = true;
}

void MainUI::closeEvent(QCloseEvent* event)
{
    m_timer->stop();

    emit closed();
    event->accept();
}

void MainUI::ChangeLanguage(const QMap<QString, QString>& moldwors, const QMap<QString, QString>& sensorwors)
{
    m_moldwors = moldwors;
    m_sensorwors = sensorwors;
    //ui->retranslateUi(this);
}

void MainUI::ChangedShowRecordT(ERecordType type)
{
    if (recordtype != type)
    {
        recordtype = type;
        refreshWidget();
    }
}

void MainUI::refreshWidget()
{
    if (recordtype == ERT_TimeInterval)
    {//更新间隔显示,如果配置界面改了,此处更新即可
        ETimeInterval ti = (ETimeInterval)m_pthis->signal_GetTimeInterval();
        if (recordtype == ERT_Shift)
        {
            UpdateTimeInterval(ti);
        }
    }
    //初始化界面状态
    ui->tb_record->setRowCount(0);
    ui->tb_record->setColumnCount(0);
    ui->l_error->setText(tr("No Latest Data"));
    ui->ledt_date->setText("");
    ui->ledt_starttime->setText("");
    ui->ledt_endtime->setText("");
    updated = false;
}

void MainUI::on_btn_to_query_clicked()
{
    m_pthis->OnChangeUI(EUI_QueryData);
}

void MainUI::on_btn_to_settings_clicked()
{
    m_pthis->OnChangeUI(EUI_Settings);
}

void MainUI::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    switch (event->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        if (strTitle != "")
        {
            setWindowTitle(strTitle);
        }
        break;
    default:
        break;
    }
}