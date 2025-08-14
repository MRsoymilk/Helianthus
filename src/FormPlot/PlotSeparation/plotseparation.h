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
    void setSeparationSeries(const QList<QPointF> v, const QString &name);
    void setSeparationInfo(const double &sugar, const double &salt);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_tBtnStandardCurve_clicked();

private:
    void init();
    void updateAxisRange();

private:
    Ui::PlotSeparation *ui;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart = nullptr;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_curve_mix, *m_curve_sugar, *m_curve_salt;
    QLineSeries *m_base_sugar, *m_base_salt;
    bool m_showBase;
};

#endif // PLOTSEPARATION_H
