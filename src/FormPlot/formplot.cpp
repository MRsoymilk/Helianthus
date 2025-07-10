#include "formplot.h"
#include "PlotData/plotdata.h"
#include "PlotHistory/plothistory.h"
#include "ui_formplot.h"

FormPlot::FormPlot(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlot)
{
    ui->setupUi(this);
    init();
}

FormPlot::~FormPlot()
{
    delete ui;
}

void FormPlot::onSerialDataReceived(const QByteArray &data24)
{
    emit processSerialData(data24);
}

void FormPlot::updatePlot(const QList<QPointF> &v24,
                          const double &xMin,
                          const double &xMax,
                          const double &yMin,
                          const double &yMax)
{
    m_series24->replace(v24);
    m_series24->setName("curve24_bit");

    m_axisX->setRange(xMin, xMax);
    if (m_autoZoom) {
        double padding = (yMax - yMin) * 0.1;
        if (padding == 0) {
            padding = 0.1;
        }
        m_axisY->setRange(yMin - padding, yMax + padding);
    } else {
        m_axisY->setRange(m_fixedYMin, m_fixedYMax);
    }
    emit send2PlotHistory(v24);
}

void FormPlot::updateTable(const QVector<double> &v24, const QVector<qint32> &raw24)
{
    emit send2PlotData(v24, raw24);
}

void FormPlot::init()
{
    m_series24 = new QLineSeries();
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

    m_chart->addSeries(m_series24);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_series24->attachAxis(m_axisX);
    m_series24->attachAxis(m_axisY);
    m_series24->setColor(Qt::blue);

    m_axisX->setTitleText("Index");
    m_axisX->setRange(0, 0.2);
    m_axisY->setTitleText("Voltage (V)");
    m_axisY->setRange(m_fixedYMin, m_fixedYMax);

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    for (QLegendMarker *marker : m_chart->legend()->markers()) {
        QObject::connect(marker, &QLegendMarker::clicked, [=]() {
            QAbstractSeries *series = marker->series();
            bool visible = series->isVisible();
            series->setVisible(!visible);
            marker->setVisible(true);
            marker->setLabelBrush(visible ? Qt::gray : Qt::black);
        });
    }

    m_chart->setTitle("Live Waveform");
    m_view = new MyChartView(m_chart);
    m_view->setRenderHint(QPainter::Antialiasing);
    ui->stackedWidget->addWidget(m_view);
    ui->stackedWidget->setCurrentWidget(m_view);

    ui->tBtnZoom->setChecked(m_autoZoom);
    m_showData = false;
    m_showHistory = false;
    ui->tBtnData->setChecked(m_showData);
    ui->tBtnHistory->setChecked(m_showHistory);
    ui->tBtnZoom->setCheckable(true);
    ui->tBtnHistory->setCheckable(true);
    ui->tBtnData->setCheckable(true);

    m_plotData = new PlotData;
    connect(this, &FormPlot::send2PlotData, m_plotData, &PlotData::updateTable);
    connect(m_plotData, &PlotData::plotDataClose, this, &FormPlot::onPlotDataClose);

    m_plotHistory = new PlotHistory;
    connect(this, &FormPlot::send2PlotHistory, m_plotHistory, &PlotHistory::updateChart);
    connect(m_plotHistory, &PlotHistory::plotHistoryClose, this, &FormPlot::onPlotHistoryClose);
}

void FormPlot::on_tBtnZoom_clicked()
{
    m_autoZoom = !m_autoZoom;
    ui->tBtnZoom->setChecked(m_autoZoom);
}

void FormPlot::wheelEvent(QWheelEvent *event)
{
    if (m_autoZoom) {
        on_tBtnZoom_clicked();
    }
    if (!m_autoZoom && (event->modifiers() & Qt::ControlModifier)) {
        int delta = event->angleDelta().y();
        double factor = (delta > 0) ? 0.9 : 1.1;

        double center = (m_fixedYMin + m_fixedYMax) / 2.0;
        double range = (m_fixedYMax - m_fixedYMin) * factor / 2.0;

        m_fixedYMin = center - range;
        m_fixedYMax = center + range;

        m_axisY->setRange(m_fixedYMin, m_fixedYMax);
    } else {
        QWidget::wheelEvent(event);
    }
}

void FormPlot::on_tBtnHistory_clicked()
{
    m_showHistory = !m_showHistory;
    if (m_showHistory) {
        m_plotHistory->show();
    } else {
        m_plotHistory->close();
    }
}

void FormPlot::on_tBtnData_clicked()
{
    m_showData = !m_showData;
    if (m_showData) {
        m_plotData->show();
    } else {
        m_plotData->close();
    }
}

void FormPlot::onPlotHistoryClose()
{
    m_showHistory = false;
    ui->tBtnHistory->setChecked(false);
}

void FormPlot::onPlotDataClose()
{
    m_showData = false;
    ui->tBtnData->setChecked(false);
}
