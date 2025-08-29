#include "plotpostprocess.h"
#include "ui_plotpostprocess.h"

PlotPostProcess::PlotPostProcess(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlotPostProcess)
{
    ui->setupUi(this);
    init();
}

PlotPostProcess::~PlotPostProcess()
{
    delete ui;
}

void PlotPostProcess::updateChart(const QVector<double> &v)
{
    QList<QPointF> points;
    double min = v[0];
    double max = v[0];
    for (int i = 0; i < v.size(); ++i) {
        QPointF point;
        point.setX(i);
        point.setY(v[i]);
        points.push_back(point);
        max = std::max(max, v[i]);
        min = std::min(min, v[i]);
    }
    m_series->replace(points);
    m_axisY->setRange(min, max);
    m_axisX->setRange(0, v.size());
}

void PlotPostProcess::updateChart(const QVector<double> &freq, const QVector<double> &magnitude)
{
    if (freq.size() != magnitude.size() || freq.isEmpty()) {
        return;
    }

    QList<QPointF> points;
    double min = magnitude[0];
    double max = magnitude[0];

    for (int i = 0; i < freq.size(); ++i) {
        points.push_back(QPointF(freq[i], magnitude[i]));
        max = std::max(max, magnitude[i]);
        min = std::min(min, magnitude[i]);
    }

    m_series->replace(points);                    // 更新曲线
    m_axisX->setRange(freq.first(), freq.last()); // x轴显示频率范围
    m_axisY->setRange(min, max);                  // y轴显示幅值范围
}

void PlotPostProcess::init()
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
    m_chart->setTitle("PostProcess");

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(m_chartView);

    // m_chartView->setContextMenuPolicy(Qt::CustomContextMenu);

    // connect(m_chartView, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos) {
    //     QMenu menu(this);
    //     QAction *clearAction = menu.addAction("Clear");
    //     connect(clearAction, &QAction::triggered, this, &PlotHistory::clearData);
    //     menu.exec(m_chartView->mapToGlobal(pos));
    // });
}
