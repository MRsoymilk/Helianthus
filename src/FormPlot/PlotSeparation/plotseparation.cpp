#include "plotseparation.h"
#include "g_define.h"
#include "ui_plotseparation.h"

PlotSeparation::PlotSeparation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlotSeparation)
{
    ui->setupUi(this);
    init();
}

PlotSeparation::~PlotSeparation()
{
    delete ui;
}

void PlotSeparation::updateAxisRange()
{
    // X 轴范围统一用所有曲线的最大最小 X
    double globalMinX = std::numeric_limits<double>::max();
    double globalMaxX = std::numeric_limits<double>::lowest();

    for (const auto &series : m_curveMap) {
        if (series && !series->points().isEmpty()) {
            globalMinX = std::min(globalMinX, series->points().first().x());
            globalMaxX = std::max(globalMaxX, series->points().last().x());
        }
    }

    if (globalMinX < globalMaxX) {
        m_axisX->setRange(globalMinX, globalMaxX);
    }

    // Y 轴范围根据 m_mapYmin / m_mapYmax 取整体最小最大
    if (!m_mapYmin.isEmpty() && !m_mapYmax.isEmpty()) {
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();

        for (auto it = m_mapYmin.begin(); it != m_mapYmin.end(); ++it) {
            if (!m_showBase) {
                if (it.key().endsWith("_base")) {
                    continue;
                }
            }
            minY = std::min(minY, it.value());
        }
        for (auto it = m_mapYmax.begin(); it != m_mapYmax.end(); ++it) {
            if (!m_showBase) {
                if (it.key().endsWith("_base")) {
                    continue;
                }
            }
            maxY = std::max(maxY, it.value());
        }

        if (minY < maxY) {
            m_axisY->setRange(minY * 0.9, maxY * 1.1);
        }
    }
}

void PlotSeparation::setSeparationSeries(const QList<QPointF> &v,
                                         const QString &name,
                                         const double y_min,
                                         const double y_max)
{
    QLineSeries *series = nullptr;

    if (!m_curveMap.contains(name)) {
        // 如果不存在，动态创建
        series = new QLineSeries();
        series->setName(name);
        m_chartLine->addSeries(series);
        series->attachAxis(m_axisX);
        series->attachAxis(m_axisY);
        m_curveMap[name] = series;
    } else {
        series = m_curveMap[name];
    }

    series->replace(v);
    m_mapYmin[name] = y_min;
    m_mapYmax[name] = y_max;

    updateAxisRange();
}

void PlotSeparation::setSeparationInfo(QMap<QString, double> ratios)
{
    // 遍历已有 slice，更新值
    for (auto slice : m_pie->slices()) {
        QString key = slice->label().section(":", 0, 0); // 取出 key（冒号前的部分）
        if (ratios.contains(key)) {
            double newVal = ratios.value(key);
            slice->setValue(newVal);
            slice->setLabel(QString("%1: %2%").arg(key).arg(newVal, 0, 'f', 2));
        }
    }

    // 检查新增的 key
    for (auto it = ratios.begin(); it != ratios.end(); ++it) {
        bool exists = false;
        for (auto slice : m_pie->slices()) {
            if (slice->label().startsWith(it.key() + ":")) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            auto slice = m_pie->append(it.key(), it.value());
            slice->setLabel(QString("%1: %2%").arg(it.key()).arg(it.value(), 0, 'f', 2));
            slice->setLabelVisible(false);

            connect(slice, &QPieSlice::hovered, this, [slice](bool hovered) {
                slice->setLabelVisible(hovered);
            });
        }
    }

    // 检查需要删除的 key（在 ratios 中不存在的）
    QList<QPieSlice *> toRemove;
    for (auto slice : m_pie->slices()) {
        QString key = slice->label().section(":", 0, 0);
        if (!ratios.contains(key)) {
            toRemove.append(slice);
        }
    }
    for (auto slice : toRemove) {
        m_pie->remove(slice);
    }
}

void PlotSeparation::init()
{
    m_chartLine = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_axisX->setTitleText("Index");
    m_axisY->setTitleText("Intensity");

    m_chartLine->addAxis(m_axisX, Qt::AlignBottom);
    m_chartLine->addAxis(m_axisY, Qt::AlignLeft);
    m_chartLine->legend()->setVisible(true);
    m_chartLine->legend()->setAlignment(Qt::AlignBottom);
    m_chartLine->setTitle(tr("Separation"));

    m_ViewLine = new MyChartView(m_chartLine, this);
    m_ViewLine->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(m_ViewLine);

    m_ViewLine->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showBase = false;
    ui->tBtnStandardCurve->setCheckable(true);

    m_chartPie = new QChart();
    m_pie = new QPieSeries();

    m_chartPie->addSeries(m_pie);
    m_chartPie->setTitle(tr("Proportion"));
    m_chartPie->legend()->setAlignment(Qt::AlignBottom);

    m_ViewPie = new QChartView(m_chartPie);
    m_ViewPie->setRenderHint(QPainter::Antialiasing);
    ui->gLayProportion->addWidget(m_ViewPie);
}

void PlotSeparation::closeEvent(QCloseEvent *event)
{
    emit windowClose();
}

void PlotSeparation::on_tBtnStandardCurve_clicked()
{
    m_showBase = !m_showBase;
    ui->tBtnStandardCurve->setChecked(m_showBase);
    if (m_showBase) {
        emit sendSeparationStandard();
    }
    for (auto it = m_curveMap.begin(); it != m_curveMap.end(); ++it) {
        if (it.key().endsWith("_base")) {
            it.value()->setVisible(m_showBase);
        }
    }
}
