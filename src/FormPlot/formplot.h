#ifndef FORMPLOT_H
#define FORMPLOT_H

#include <QWidget>
#include "mychartview.h"

namespace Ui {
class FormPlot;
}

class PlotData;
class PlotHistory;

class FormPlot : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlot(QWidget *parent = nullptr);
    ~FormPlot();

signals:
    void processSerialData(const QByteArray &data24);
    void send2PlotData(const QVector<double> &v24, const QVector<qint32> &raw24);
    void send2PlotHistory(const QList<QPointF> &v24);

public slots:
    void onSerialDataReceived(const QByteArray &data24);
    void updatePlot(const QList<QPointF> &v24,
                    const double &xMin,
                    const double &xMax,
                    const double &yMin,
                    const double &yMax);
    void updateTable(const QVector<double> &v24, const QVector<qint32> &raw24);

protected:
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void on_tBtnZoom_clicked();
    void on_tBtnHistory_clicked();
    void on_tBtnData_clicked();
    void onPlotHistoryClose();
    void onPlotDataClose();

private:
    void init();

private:
    Ui::FormPlot *ui;
    MyChartView *m_view;
    QChart *m_chart;
    QLineSeries *m_series24;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    double m_fixedYMin = -2.5;
    double m_fixedYMax = 2.5;
    bool m_autoZoom;
    bool m_showHistory;
    bool m_showData;
    PlotData *m_plotData;
    PlotHistory *m_plotHistory;
};

#endif // FORMPLOT_H
