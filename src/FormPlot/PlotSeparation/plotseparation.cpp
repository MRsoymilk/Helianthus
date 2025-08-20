#include "plotseparation.h"
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
    qreal minX = std::numeric_limits<qreal>::max();
    qreal maxX = std::numeric_limits<qreal>::lowest();
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();

    for (auto series : {m_curve_mix, m_curve_sugar, m_curve_salt, m_base_sugar, m_base_salt}) {
        if (!series)
            continue;
        for (const QPointF &pt : series->points()) {
            minX = std::min(minX, pt.x());
            maxX = std::max(maxX, pt.x());
            minY = std::min(minY, pt.y());
            maxY = std::max(maxY, pt.y());
        }
    }

    if (minX < maxX) {
        m_axisX->setRange(minX, maxX);
    }
    if (minY < maxY) {
        m_axisY->setRange(minY * 0.9, maxY * 1.1);
    }
}

void PlotSeparation::setSeparationSeries(const QList<QPointF> v, const QString &name)
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
    updateAxisRange();
    m_chartLine->update();
}

void PlotSeparation::setSeparationInfo(const double &sugar, const double &salt, const double &powder)
{
    if (m_pie->slices().size() >= 2) {
        m_pie->slices().at(0)->setValue(sugar);  // Sugar
        m_pie->slices().at(1)->setValue(salt);   // Salt
        m_pie->slices().at(2)->setValue(powder); // Powder

        for (auto slice : m_pie->slices()) {
            slice->setLabel(QString("%1: %2%").arg(slice->label()).arg(slice->value(), 0, 'f', 2));
            slice->setLabelVisible(true);
        }
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

    for (auto slice : m_pie->slices()) {
        slice->setLabel(QString("%1: %2%").arg(slice->label()).arg(slice->value(), 0, 'f', 2));
        slice->setLabelVisible(true);
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
