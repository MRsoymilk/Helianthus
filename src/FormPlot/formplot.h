#ifndef FORMPLOT_H
#define FORMPLOT_H

#include <QWidget>
#include "mychartview.h"

namespace Ui {
class FormPlot;
}

class PlotData;
class PlotHistory;
class PlotFilter;
class PlotSeparation;

class FormPlot : public QWidget
{
    Q_OBJECT
public:
    enum class METHOD { REFLECTION, TRANSMISSION };

public:
    explicit FormPlot(QWidget *parent = nullptr);
    ~FormPlot();

signals:
    void processSerialData(const QByteArray &data24);
    void send2PlotData(const QVector<double> &v24, const QVector<qint32> &raw24);
    void send2PlotHistory(const QList<QPointF> &v24);
    void sendPlotMethod(int method);
    void sendPlotStartEnd(int start, int end);
    void sendPlotIntegration(int ms);
    void sendPlotBaselineSub(bool isDo);
    void sendPlotClassify(bool isDo);
    void sendFilter(const double &average, const double &distance);
    void sendPlotSeparation(bool isDo);
    void sendSeparationStandard();

public slots:
    void onSerialDataReceived(const QByteArray &data24);
    void updatePlot(const QList<QPointF> &v24,
                    const double &xMin,
                    const double &xMax,
                    const double &yMin,
                    const double &yMax);
    void updateTable(const QVector<double> &v24, const QVector<qint32> &raw24);
    void onSendParams();
    void onSendLineInfo(const double &val_average, const double &val_distance);
    void onSendSeparationSeries(const QList<QPointF> v, const QString &name);
    void onSendSeparationInfo(const double &sugar, const double &salt);

protected:
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void on_tBtnZoom_clicked();
    void on_tBtnHistory_clicked();
    void on_tBtnData_clicked();
    void onPlotHistoryClose();
    void onPlotDataClose();
    void on_tBtnBaselineSubtraction_clicked();
    void on_comboBoxMethod_currentTextChanged(const QString &text);
    void on_spinBoxStart_valueChanged(int val);
    void on_spinBoxEnd_valueChanged(int val);
    void on_spinBox_ms_valueChanged(int val);
    void on_tBtnPicture_clicked();
    void on_tBtnClassify_clicked();
    void on_tBtnFilter_clicked();
    void on_tBtnSeparation_clicked();

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
    bool m_baseline_sub;
    bool m_classify;
    bool m_showFilter;
    bool m_showSeparation;
    PlotData *m_plotData;
    PlotHistory *m_plotHistory;
    PlotFilter *m_plotFilter;
    PlotSeparation *m_plotSeparation;
    METHOD m_method;
};

#endif // FORMPLOT_H
