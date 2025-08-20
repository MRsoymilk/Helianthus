#ifndef PLOTSEPARATION_H
#define PLOTSEPARATION_H

#include <QWidget>
#include "mychartview.h"

namespace Ui {
class PlotSeparation;
}

class PlotSeparation : public QWidget
{
    Q_OBJECT

public:
    explicit PlotSeparation(QWidget *parent = nullptr);
    ~PlotSeparation();

signals:
    void windowClose();
    void sendSeparationStandard();

public:
    void setSeparationSeries(const QList<QPointF> v,
                             const QString &name,
                             const double y_min,
                             const double y_max);
    void setSeparationInfo(const double &sugar, const double &salt, const double &powder);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_tBtnStandardCurve_clicked();

private:
    void init();
    void updateAxisRange();

private:
    Ui::PlotSeparation *ui;
    MyChartView *m_ViewLine = nullptr;
    QChartView *m_ViewPie = nullptr;
    QChart *m_chartLine = nullptr;
    QChart *m_chartPie = nullptr;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_curve_mix, *m_curve_sugar, *m_curve_salt, *m_curve_powder;
    QLineSeries *m_base_sugar, *m_base_salt, *m_base_powder;
    QPieSeries *m_pie;
    bool m_showBase;
    QMap<int, QString> m_material;
    QMap<QString, double> m_mapYmin;
    QMap<QString, double> m_mapYmax;
};

#endif // PLOTSEPARATION_H
