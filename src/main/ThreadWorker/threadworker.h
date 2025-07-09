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

signals:
    void dataForTableReady(const QVector<double> &v24, const QVector<qint32> &raw24);
    void dataForPlotReady(const QList<QPointF> &v24,
                          const double &xMin,
                          const double &xMax,
                          const double &yMin,
                          const double &yMax);
    void classificationForResult(RESULT result);
    void classificationForHistory(const QJsonObject &obj);

private:
    void processCurve24(const QByteArray &data24,
                        QVector<double> &v_voltage24,
                        QVector<qint32> &raw24,
                        double &yMin,
                        double &yMax);
    void sendPredictRequest(const QVector<double> &v_voltage24);
};

#endif // THREADWORKER_H
