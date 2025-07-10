#ifndef PLOTHISTORY_H
#define PLOTHISTORY_H

#include <QWidget>
#include "mychartview.h"

namespace Ui {
class PlotHistory;
}

class PlotHistory : public QWidget
{
    Q_OBJECT

public:
    explicit PlotHistory(QWidget *parent = nullptr);
    ~PlotHistory();

signals:
    void plotHistoryClose();

public slots:
    void updateChart(const QList<QPointF> &v24);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_tBtnPrev_clicked();
    void on_tBtnNext_clicked();
    void on_lineEditGo_editingFinished();

private:
    void fillChart(const QList<QPointF> &v24);
    void init();
    void clearData();

private:
    Ui::PlotHistory *ui;
    QList<QList<QPointF>> m_listV24;
    int m_current = 0;
    int m_total;    
    MyChartView *m_chartView = nullptr;
    QChart *m_chart = nullptr;
    QLineSeries *m_series;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
};

#endif // PLOTHISTORY_H
