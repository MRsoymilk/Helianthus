#include "plotfilter.h"
#include "g_define.h"
#include "ui_plotfilter.h"

PlotFilter::PlotFilter(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlotFilter)
{
    ui->setupUi(this);
    m_filter = false;
    ui->btnFilter->setCheckable(true);
    ui->lineEditFilterAverage->setText(SETTING_CONFIG_GET(CFG_GROUP_FILTER, CFG_FILTER_AVERAGE, ""));
    ui->lineEditFilterDistance->setText(
        SETTING_CONFIG_GET(CFG_GROUP_FILTER, CFG_FILTER_DISTANCE, ""));
}

PlotFilter::~PlotFilter()
{
    delete ui;
}

void PlotFilter::setAverage(const double &val)
{
    m_min_avg = std::min(m_min_avg, val);
    m_max_avg = std::max(m_max_avg, val);
    ui->lineEditValAverage->setText(QString::number(val));
    ui->labelValAvgMin->setText(QString("%1").arg(m_min_avg));
    ui->labelValAvgMax->setText(QString("%1").arg(m_max_avg));
}

void PlotFilter::setDistance(const double &val)
{
    m_min_dist = std::min(m_min_dist, val);
    m_max_dist = std::max(m_max_dist, val);
    ui->lineEditValDistance->setText(QString::number(val));
    ui->labelValDistMin->setText(QString("%1").arg(m_min_dist));
    ui->labelValDistMax->setText(QString("%1").arg(m_max_dist));
}

void PlotFilter::resetMinMax()
{
    m_min_avg = m_min_dist = std::numeric_limits<double>::max();
    m_max_avg = m_max_dist = std::numeric_limits<double>::min();
    ui->labelValAvgMin->setText(QString("%1").arg(m_min_avg));
    ui->labelValAvgMax->setText(QString("%1").arg(m_max_avg));
    ui->labelValDistMin->setText(QString("%1").arg(m_min_dist));
    ui->labelValDistMax->setText(QString("%1").arg(m_max_dist));
}

void PlotFilter::on_btnFilter_clicked()
{
    m_filter = !m_filter;
    ui->btnFilter->setChecked(m_filter);
    if (m_filter) {
        ui->btnFilter->setText("On Filter");
        QString avg = ui->lineEditFilterAverage->text();
        QString dist = ui->lineEditFilterDistance->text();
        SETTING_CONFIG_SET(CFG_GROUP_FILTER, CFG_FILTER_AVERAGE, avg);
        SETTING_CONFIG_SET(CFG_GROUP_FILTER, CFG_FILTER_DISTANCE, dist);
        emit sendFilter(avg.toDouble(), dist.toDouble());
    } else {
        ui->btnFilter->setText("Filter");
    }
}

void PlotFilter::closeEvent(QCloseEvent *event)
{
    emit windowClose();
    QWidget::closeEvent(event);
}
