#ifndef PLOTPOSTPROCESS_H
#define PLOTPOSTPROCESS_H

#include <QWidget>
#include "mychartview.h"

namespace Ui {
class PlotPostProcess;
}

class PlotPostProcess : public QWidget
{
    Q_OBJECT

public:
    explicit PlotPostProcess(QWidget *parent = nullptr);
    ~PlotPostProcess();
    void updateChart(const QVector<double> &v);
    void updateChart(const QVector<double> &freq, const QVector<double> &magnitude);

private:
    void init();

private:
    Ui::PlotPostProcess *ui;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart = nullptr;
    QLineSeries *m_series;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
};

#endif // PLOTPOSTPROCESS_H
