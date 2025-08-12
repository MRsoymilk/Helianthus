#ifndef PLOTFILTER_H
#define PLOTFILTER_H

#include <QWidget>

namespace Ui {
class PlotFilter;
}

class PlotFilter : public QWidget
{
    Q_OBJECT

public:
    explicit PlotFilter(QWidget *parent = nullptr);
    ~PlotFilter();
    void setAverage(const double &val);
    void setDistance(const double &val);
    void resetMinMax();

signals:
    void sendFilter(const double &average, const double &distance);
    void windowClose();

private slots:
    void on_btnFilter_clicked();

private:
    Ui::PlotFilter *ui;
    double m_min_avg;
    double m_max_avg;
    double m_min_dist;
    double m_max_dist;
    bool m_filter;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // PLOTFILTER_H
