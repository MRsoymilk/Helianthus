#ifndef THREADWORKER_H
#define THREADWORKER_H

#include <QObject>
#include "g_define.h"

class ThreadWorker : public QObject
{
    Q_OBJECT
public:
    explicit ThreadWorker(QObject *parent = nullptr);
    ~ThreadWorker();

public slots:
    void processData(const QByteArray &data24);
    void onOtoRequest(bool going, const QString &url);
    void onPlotMethod(int method);
    void onPlotStartEnd(int start, int end);
    void onPlotIntegration(int ms);
    void onPlotSubBaseline(bool isDo);
    void onPlotClassify(bool isDo);

signals:
    void dataForTableReady(const QVector<double> &v24, const QVector<qint32> &raw24);
    void dataForPlotReady(const QList<QPointF> &v24,
                          const double &xMin,
                          const double &xMax,
                          const double &yMin,
                          const double &yMax);
    void classificationForResult(RESULT result);
    void classificationForHistory(const QJsonObject &obj);
    void otoRequestRaw(const QJsonObject &obj);
    void otoBaselineProgress(const QString &progress);

private:
    void processCurve24(const QByteArray &data24,
                        QVector<double> &v_voltage24,
                        QVector<qint32> &raw24,
                        double &yMin,
                        double &yMax);
    void sendPredictRequest(const QVector<double> &v_voltage24);
    void otoRequest();

    QTimer *m_timer;
    QString m_url;
    QString m_base_url;
    int m_plot_start, m_plot_end;
    int m_plot_method;
    int m_plot_integration;
    bool m_b_plot_sub_baseline;
    QMap<int, double> m_map_plot_baseline;
    int m_plot_baseline_count;
    bool m_plot_classify;
};

#endif // THREADWORKER_H
