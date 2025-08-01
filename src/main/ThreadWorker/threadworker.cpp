#include "threadworker.h"
#include "g_define.h"

#include <QJsonArray>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "myhttp.h"

ThreadWorker::ThreadWorker(QObject *parent)
    : QObject(parent)
{}

ThreadWorker::~ThreadWorker() {}

void ThreadWorker::processCurve24(const QByteArray &data24,
                                  QVector<double> &v_voltage24,
                                  QVector<qint32> &raw24,
                                  double &yMin,
                                  double &yMax)
{
    QByteArray payload = data24.mid(5, data24.size() - 7);
    if (payload.size() % 3 != 0) {
        LOG_WARN("Invalid data length: {}", payload.size());
        return;
    }
    QByteArray filteredPayload;
    for (int i = 0; i < payload.size(); i += 3) {
        quint32 value = (static_cast<quint8>(payload[i]) << 16)
                        | (static_cast<quint8>(payload[i + 1]) << 8)
                        | static_cast<quint8>(payload[i + 2]);
        filteredPayload.append(payload.mid(i, 3));
    }

    payload = filteredPayload;

    int numPoints = payload.size() / 3;

    for (int i = 0; i < numPoints; ++i) {
        int idx = i * 3;
        quint32 raw = (quint8) payload[idx] << 16 | (quint8) payload[idx + 1] << 8
                      | (quint8) payload[idx + 2];

        qint32 signedRaw = static_cast<qint32>(raw);
        double voltage;

        if (signedRaw & 0x800000) {
            signedRaw |= 0xFF000000;
        }
        raw24.push_back(signedRaw);
        voltage = signedRaw / double(1 << 23) * 2.5;

        if (voltage < yMin)
            yMin = voltage;
        if (voltage > yMax)
            yMax = voltage;

        v_voltage24.push_back(voltage);
    }
}

void ThreadWorker::processData(const QByteArray &data24)
{
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    QVector<double> v_voltage24;
    QVector<qint32> raw24;

    processCurve24(data24, v_voltage24, raw24, yMin, yMax);

    double xMin = 980;
    double xMax = 1999.0;

    QList<QPointF> out24;

    xMin = 980;
    xMax = std::min(xMin + v_voltage24.size(), xMin + 660);
    for (int i = 0; i < v_voltage24.size(); ++i) {
        out24.push_back({static_cast<double>(i + xMin), v_voltage24[i]});
    }

    emit dataForTableReady(v_voltage24, raw24);
    emit dataForPlotReady(out24, xMin, xMax, yMin, yMax);

    QJsonObject inputObj;
    QJsonArray signalArray;
    for (double val : v_voltage24)
        signalArray.append(val);
    inputObj["signal"] = signalArray;

    sendPredictRequest(v_voltage24);
}

void ThreadWorker::sendPredictRequest(const QVector<double> &v_voltage24)
{
    QJsonObject inputObj;
    QJsonArray signalArray;
    for (double val : v_voltage24)
        signalArray.append(val);
    inputObj["signal"] = signalArray;

    MyHttp *http = new MyHttp(this);
    QUrl url("http://192.168.123.233:5010/knn_predict");

    connect(http, &MyHttp::jsonResponse, this, [=](const QJsonObject &obj) {
        QString res = obj["result"].toString();

        if (res == "橄榄油")
            emit classificationForResult(RESULT::OliveOil);
        else if (res == "水")
            emit classificationForResult(RESULT::Water);
        else if (res == "空")
            emit classificationForResult(RESULT::Empty);
        else if (res == "糖水")
            emit classificationForResult(RESULT::TongSui);
        else if (res == "芝麻油")
            emit classificationForResult(RESULT::SesameOil);
        else if (res == "葵花籽油")
            emit classificationForResult(RESULT::SunflowerOil);
        else if (res == "75酒精")
            emit classificationForResult(RESULT::Alcohol75);
        else if (res == "C2H4O2")
            emit classificationForResult(RESULT::C2H4O2);
        else if (res == "C2H6O")
            emit classificationForResult(RESULT::C2H6O);
        else if (res == "玉米油")
            emit classificationForResult(RESULT::CornOil);
        else if (res == "空瓶")
            emit classificationForResult(RESULT::EmptyBottle);

        emit classificationForHistory(obj);
        http->deleteLater();
    });

    connect(http, &MyHttp::httpError, this, [=](const QString &err) {
        qWarning() << "HTTP error:" << err;
        http->deleteLater();
    });

    http->postJson(url, inputObj);
}
