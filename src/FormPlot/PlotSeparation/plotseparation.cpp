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
    } else if (name == "StandardSugar") {
        m_base_sugar->replace(v);
    } else if (name == "StandardSalt") {
        m_base_salt->replace(v);
    }
    updateAxisRange();
    m_chart->update();
}

void PlotSeparation::setSeparationInfo(const double &sugar, const double &salt)
{
    ui->label->setText(tr("sugar: %1%\nsalt: %2%").arg(sugar, 0, 'f', 2).arg(salt, 0, 'f', 2));
}

void PlotSeparation::init()
{
    m_chart = new QChart();
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_axisX->setTitleText("Index");
    m_axisY->setTitleText("Intensity");

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setTitle("Separation");

    m_curve_mix = new QLineSeries();
    m_curve_sugar = new QLineSeries();
    m_curve_salt = new QLineSeries();

    m_base_sugar = new QLineSeries();
    m_base_salt = new QLineSeries();

    m_chart->addSeries(m_curve_mix);
    m_curve_mix->setName("mix");
    m_curve_mix->attachAxis(m_axisX);
    m_curve_mix->attachAxis(m_axisY);

    m_chart->addSeries(m_curve_sugar);
    m_curve_sugar->setName("sugar");
    m_curve_sugar->attachAxis(m_axisX);
    m_curve_sugar->attachAxis(m_axisY);

    m_chart->addSeries(m_curve_salt);
    m_curve_salt->setName("salt");
    m_curve_salt->attachAxis(m_axisX);
    m_curve_salt->attachAxis(m_axisY);

    m_chart->addSeries(m_base_sugar);
    m_base_sugar->setName("base sugar");
    m_base_sugar->attachAxis(m_axisX);
    m_base_sugar->attachAxis(m_axisY);

    m_chart->addSeries(m_base_salt);
    m_base_salt->setName("base salt");
    m_base_salt->attachAxis(m_axisX);
    m_base_salt->attachAxis(m_axisY);

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(m_chartView);

    m_chartView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showBase = false;
    ui->tBtnStandardCurve->setCheckable(true);
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
