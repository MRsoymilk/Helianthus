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
    void onPlotSeparation(bool isDo);
    void onSendFilter(const double &average, const double &distance);
    void onPlotSeparationStandard();
    void onSelfTrainRecord(bool isDo);

signals:
    void dataForTableReady(const QVector<double> &v24, const QVector<qint32> &raw24);
    void dataForPlotReady(const QList<QPointF> &v24,
                          const double &xMin,
                          const double &xMax,
                          const double &yMin,
                          const double &yMax);
    void dataForSelfTrain(const QList<QPointF> &v24,
                          const double &xMin,
                          const double &xMax,
                          const double &yMin,
                          const double &yMax);
    void classificationForResult(RESULT result, const QString &name);
    void classificationForHistory(const QJsonObject &obj);
    void otoRequestRaw(const QJsonObject &obj);
    void otoBaselineProgress(const QString &progress);
    void sendLineInfo(const double &val_average, const double &val_distance);
    void sendSeparationSeries(const QList<QPointF> v,
                              const QString &name,
                              const double &y_min,
                              const double &y_max);
    void sendSeparationInfo(const double &surgar, const double &salt, const double &powder);

private:
    void processCurve24(const QByteArray &data24,
                        QVector<double> &v_voltage24,
                        QVector<qint32> &raw24,
                        double &yMin,
                        double &yMax);
    void sendPredictRequest(const QVector<double> &v_voltage24);
    void sendSeparationRequest(const QVector<double> &v_voltage24);
    void otoRequest();

private:
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
    bool m_plot_separation;
    bool m_self_train_record;
    double m_filter_average;
    double m_filter_distance;
};

#endif // THREADWORKER_H
