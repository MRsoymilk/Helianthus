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

    for (const auto &series : {m_curve_mix,
                               m_curve_sugar,
                               m_curve_salt,
                               m_curve_powder,
                               m_base_sugar,
                               m_base_salt,
                               m_base_powder}) {
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
            minY = std::min(minY, it.value());
        }
        for (auto it = m_mapYmax.begin(); it != m_mapYmax.end(); ++it) {
            maxY = std::max(maxY, it.value());
        }

        if (minY < maxY) {
            m_axisY->setRange(minY * 0.9, maxY * 1.1);
        }
    }
}

void PlotSeparation::setSeparationSeries(const QList<QPointF> v,
                                         const QString &name,
                                         const double y_min,
                                         const double y_max)
{
    if (name == "Mix") {
        m_curve_mix->replace(v);
    } else if (name == "Sugar") {
        m_curve_sugar->replace(v);
    } else if (name == "Salt") {
        m_curve_salt->replace(v);
    } else if (name == "Powder") {
        m_curve_powder->replace(v);
    } else if (name == "StandardSugar") {
        m_base_sugar->replace(v);
    } else if (name == "StandardSalt") {
        m_base_salt->replace(v);
    } else if (name == "StandardPowder") {
        m_base_powder->replace(v);
    }
    m_mapYmin[name] = y_min;
    m_mapYmax[name] = y_max;
    updateAxisRange();
    m_chartLine->update();
}

void PlotSeparation::setSeparationInfo(const double &sugar, const double &salt, const double &powder)
{
    if (m_pie->slices().size() >= 3) {
        m_pie->slices().at(0)->setValue(sugar);  // Sugar
        m_pie->slices().at(1)->setValue(salt);   // Salt
        m_pie->slices().at(2)->setValue(powder); // Powder

        m_pie->slices().at(0)->setLabel(QString("Sugar: %1%").arg(sugar, 0, 'f', 2));
        m_pie->slices().at(1)->setLabel(QString("Salt: %1%").arg(salt, 0, 'f', 2));
        m_pie->slices().at(2)->setLabel(QString("Powder: %1%").arg(powder, 0, 'f', 2));
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

    m_curve_mix = new QLineSeries();
    m_curve_sugar = new QLineSeries();
    m_curve_salt = new QLineSeries();
    m_curve_powder = new QLineSeries();

    m_base_sugar = new QLineSeries();
    m_base_salt = new QLineSeries();
    m_base_powder = new QLineSeries();

    m_chartLine->addSeries(m_curve_mix);
    m_curve_mix->setName("mix");
    m_curve_mix->attachAxis(m_axisX);
    m_curve_mix->attachAxis(m_axisY);

    m_chartLine->addSeries(m_curve_sugar);
    m_curve_sugar->setName("sugar");
    m_curve_sugar->attachAxis(m_axisX);
    m_curve_sugar->attachAxis(m_axisY);

    m_chartLine->addSeries(m_curve_salt);
    m_curve_salt->setName("salt");
    m_curve_salt->attachAxis(m_axisX);
    m_curve_salt->attachAxis(m_axisY);

    m_chartLine->addSeries(m_curve_powder);
    m_curve_powder->setName("powder");
    m_curve_powder->attachAxis(m_axisX);
    m_curve_powder->attachAxis(m_axisY);

    m_chartLine->addSeries(m_base_sugar);
    m_base_sugar->setName("base sugar");
    m_base_sugar->attachAxis(m_axisX);
    m_base_sugar->attachAxis(m_axisY);

    m_chartLine->addSeries(m_base_salt);
    m_base_salt->setName("base salt");
    m_base_salt->attachAxis(m_axisX);
    m_base_salt->attachAxis(m_axisY);

    m_chartLine->addSeries(m_base_powder);
    m_base_powder->setName("base powder");
    m_base_powder->attachAxis(m_axisX);
    m_base_powder->attachAxis(m_axisY);

    m_ViewLine = new MyChartView(m_chartLine, this);
    m_ViewLine->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(m_ViewLine);

    m_ViewLine->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showBase = false;
    ui->tBtnStandardCurve->setCheckable(true);

    m_material[0] = "Sugar";
    m_material[1] = "Salt";
    m_material[2] = "Powder";
    m_chartPie = new QChart();
    m_pie = new QPieSeries();
    m_pie->append(m_material[0], 1.0 / 3 * 100);
    m_pie->append(m_material[1], 1.0 / 3 * 100);
    m_pie->append(m_material[2], 1.0 / 3 * 100);

    m_pie->slices().at(0)->setLabel(QString("Sugar: %1%").arg(1.0 / 3 * 100, 0, 'f', 2));
    m_pie->slices().at(1)->setLabel(QString("Salt: %1%").arg(1.0 / 3 * 100, 0, 'f', 2));
    m_pie->slices().at(2)->setLabel(QString("Powder: %1%").arg(1.0 / 3 * 100, 0, 'f', 2));
    for (auto slice : m_pie->slices()) {
        slice->setLabelVisible(false);

        // 连接悬浮信号
        connect(slice, &QPieSlice::hovered, this, [slice](bool hovered) {
            slice->setLabelVisible(hovered);
        });
    }
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

    m_base_salt->setVisible(m_showBase);
    m_base_sugar->setVisible(m_showBase);
}
