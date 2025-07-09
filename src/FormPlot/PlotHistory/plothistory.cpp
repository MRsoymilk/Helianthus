#include "plothistory.h"
#include "ui_plothistory.h"

PlotHistory::PlotHistory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlotHistory)
{
    ui->setupUi(this);
    init();
}

PlotHistory::~PlotHistory()
{
    delete ui;
}

void PlotHistory::init()
{
    m_series = new QLineSeries();
    m_series->setColor(Qt::blue);
    m_chart = new QChart();
    m_chart->addSeries(m_series);
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_axisX->setTitleText("Index");
    m_axisX->setRange(0, 0.2);
    m_axisY->setTitleText("Voltage (V)");
    m_axisY->setRange(0, 0.2);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setTitle("History Plot");

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayHistoryPlot->addWidget(m_chartView);
}

void PlotHistory::updateChart(const QList<QPointF> &v24)
{
    if (this->isVisible()) {
        m_listV24.push_back(v24);
        m_current = m_total = m_listV24.size();
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));
        fillChart(v24);
    }
}

void PlotHistory::fillChart(const QList<QPointF> &v24)
{
    if (v24.empty()) {
        return;
    }
    m_series->replace(v24);

    qreal minX = v24[0].x(), maxX = v24[0].x();
    qreal minY = v24[0].y(), maxY = v24[0].y();
    for (const QPointF &point : v24) {
        minX = qMin(minX, point.x());
        maxX = qMax(maxX, point.x());
        minY = qMin(minY, point.y());
        maxY = qMax(maxY, point.y());
    }
    m_axisX->setRange(minX, maxX);
    m_axisY->setRange(minY, maxY * 1.1);
}

void PlotHistory::closeEvent(QCloseEvent *event)
{
    m_listV24.clear();
    emit plotHistoryClose();
    QWidget::closeEvent(event);
}

void PlotHistory::on_tBtnPrev_clicked()
{
    if (m_current - 1 > 0) {
        m_current--;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));
        fillChart(m_listV24[m_current - 1]);
    }
}

void PlotHistory::on_tBtnNext_clicked()
{
    if (m_current + 1 <= m_total) {
        m_current++;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));
        fillChart(m_listV24[m_current - 1]);
    }
}

void PlotHistory::on_lineEditGo_editingFinished()
{
    int val = ui->lineEditGo->text().toInt();
    if (val > 0 && val <= m_total) {
        m_current = val;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));
        fillChart(m_listV24[m_current - 1]);
    }
}
