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
{
    m_plot_separation = false;
    m_plot_classify = false;
    m_plot_start = m_plot_end = 0;
    m_plot_integration = 0;
    m_plot_method = 0;
    m_map_plot_baseline.clear();
    m_plot_baseline_count = 0;
    m_b_plot_sub_baseline = false;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ThreadWorker::otoRequest);
}

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
        if (m_plot_baseline_count > 0) {
            m_map_plot_baseline[i] += v_voltage24[i];
        }
    }

    if (m_plot_baseline_count > 0) {
        --m_plot_baseline_count;
    }

    if (m_plot_start != 0) {
        xMin = m_plot_start;
    }
    if (m_plot_end != 0) {
        xMax = m_plot_end;
    }

    double val_average = 0;
    double val_distance = 0;

    if (m_b_plot_sub_baseline && m_plot_baseline_count == 0) {
        yMin = std::numeric_limits<double>::max();
        yMax = std::numeric_limits<double>::lowest();
        // REFLECTION
        if (m_plot_method == 0) {
            for (int i = 0; i < v_voltage24.size(); ++i) {
                v_voltage24[i] -= m_map_plot_baseline[i] / 10;
            }
            for (int i = 0; i < out24.size(); ++i) {
                out24[i].setY(out24[i].y() - m_map_plot_baseline[i] / 10);
                yMin = std::min(yMin, out24[i].y());
                yMax = std::max(yMax, out24[i].y());
            }
        }
        // TRANSMISSION
        else if (m_plot_method == 1) {
            for (int i = 0; i < v_voltage24.size(); ++i) {
                v_voltage24[i] = v_voltage24[i] / (m_map_plot_baseline[i] / 10.0) * 100;
            }
            for (int i = 0; i < out24.size(); ++i) {
                out24[i].setY(out24[i].y() / (m_map_plot_baseline[i] / 10.0) * 100);
                yMax = std::max(yMax, out24[i].y());
            }
            yMin = 0;
        }

        for (int i = 0; i < out24.size(); ++i) {
            // average
            val_average += out24[i].y();
            val_distance += std::abs(out24[i].y());
        }
        val_average /= v_voltage24.size();
        emit sendLineInfo(val_average, val_distance);
    }

    emit dataForTableReady(v_voltage24, raw24);
    emit dataForPlotReady(out24, xMin, xMax, yMin, yMax);

    if (m_plot_classify) {
        if (val_average > m_filter_average && val_distance > m_filter_distance) {
            sendPredictRequest(v_voltage24);
        } else {
            emit classificationForResult(RESULT::Empty, "Filtered");
        }
    }

    if (m_plot_separation) {
        sendSeparationRequest(v_voltage24);
    }
}

void ThreadWorker::onOtoRequest(bool going, const QString &url)
{
    if (going) {
        m_base_url = url;
        m_timer->start(100);
    } else {
        m_timer->stop();
    }
}

void ThreadWorker::onPlotMethod(int method)
{
    m_plot_method = method;
}

void ThreadWorker::onPlotStartEnd(int start, int end)
{
    m_plot_start = start;
    m_plot_end = end;
}

void ThreadWorker::onPlotIntegration(int ms)
{
    m_plot_integration = ms * 1000;
}

void ThreadWorker::onPlotSubBaseline(bool isDo)
{
    m_b_plot_sub_baseline = isDo;
    if (isDo) {
        m_map_plot_baseline.clear();
        m_plot_baseline_count = 10;
    }
}

void ThreadWorker::onPlotClassify(bool isDo)
{
    m_plot_classify = isDo;
}

void ThreadWorker::onPlotSeparation(bool isDo)
{
    m_plot_separation = isDo;
}

void ThreadWorker::onSendFilter(const double &average, const double &distance)
{
    m_filter_average = average;
    m_filter_distance = distance;
}

void ThreadWorker::otoRequest()
{
    MyHttp http;
    if (m_plot_integration != 0) {
        m_url = QString("%1?integration_time_us=%2").arg(m_base_url).arg(m_plot_integration);
    } else {
        m_url = m_base_url;
    }
    QJsonObject obj = http.get_sync(m_url);
    emit otoRequestRaw(obj);

    QJsonArray arr = obj["spectrum"].toArray();

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    QList<QPointF> out24;
    QVector<double> v_voltage24;
    QVector<qint32> raw24;
    for (int i = 0; i < arr.size(); ++i) {
        QJsonObject item = arr.at(i).toObject();
        for (auto it = item.begin(); it != item.end(); ++it) {
            double wavelength = it.key().toDouble();
            double intensity = it.value().toDouble();
            out24.push_back({wavelength, intensity});
            if (m_plot_baseline_count > 0) {
                m_map_plot_baseline[i] += intensity;
            }
            xMin = std::min(xMin, wavelength);
            xMax = std::max(xMax, wavelength);
            yMin = std::min(yMin, intensity);
            yMax = std::max(yMax, intensity);
            v_voltage24.push_back(intensity);
            raw24.push_back(0);
        }
    }

    if (m_plot_baseline_count > 0) {
        --m_plot_baseline_count;
    }

    if (m_plot_start != 0) {
        xMin = m_plot_start;
    }
    if (m_plot_end != 0) {
        xMax = m_plot_end;
    }

    double val_average = 0;
    double val_distance = 0;

    if (m_b_plot_sub_baseline && m_plot_baseline_count == 0) {
        yMin = std::numeric_limits<double>::max();
        yMax = std::numeric_limits<double>::lowest();
        // REFLECTION
        if (m_plot_method == 0) {
            for (int i = 0; i < v_voltage24.size(); ++i) {
                v_voltage24[i] -= m_map_plot_baseline[i] / 10;
            }
            for (int i = 0; i < out24.size(); ++i) {
                out24[i].setY(out24[i].y() - m_map_plot_baseline[i] / 10);
                yMin = std::min(yMin, out24[i].y());
                yMax = std::max(yMax, out24[i].y());
            }
        }
        // TRANSMISSION
        else if (m_plot_method == 1) {
            for (int i = 0; i < v_voltage24.size(); ++i) {
                v_voltage24[i] = v_voltage24[i] / (m_map_plot_baseline[i] / 10.0) * 100;
            }
            for (int i = 0; i < out24.size(); ++i) {
                out24[i].setY(out24[i].y() / (m_map_plot_baseline[i] / 10.0) * 100);
                yMax = std::max(yMax, out24[i].y());
            }
            yMin = 0;
        }

        for (int i = 0; i < out24.size(); ++i) {
            // average
            val_average += out24[i].y();
            val_distance += std::abs(out24[i].y());
        }
        val_average /= v_voltage24.size();
        emit sendLineInfo(val_average, val_distance);
    }

    emit dataForTableReady(v_voltage24, raw24);
    emit dataForPlotReady(out24, xMin, xMax, yMin, yMax);
    if (m_plot_classify) {
        if (val_average > m_filter_average && val_distance > m_filter_distance) {
            sendPredictRequest(v_voltage24);
        } else {
            emit classificationForResult(RESULT::Empty, "Filtered");
        }
    }
    if (m_plot_separation) {
        sendSeparationRequest(v_voltage24);
    }
}

void ThreadWorker::sendPredictRequest(const QVector<double> &v_voltage24)
{
    QJsonObject inputObj;
    QJsonArray signalArray;
    for (double val : v_voltage24)
        signalArray.append(val);
    inputObj["signal"] = signalArray;
    inputObj["method"] = m_plot_method == 0 ? "reflection" : "transmission";

    MyHttp *http = new MyHttp(this);
    QUrl url("http://192.168.123.233:5010/knn_predict");

    auto cleanup = [http]() { http->deleteLater(); };

    connect(http, &MyHttp::jsonResponse, this, [=](const QJsonObject &obj) {
        QString res = obj["result"].toString();
        if (res == "空") {
            emit classificationForResult(RESULT::Empty, res);
        } else if (res == "淀粉") {
            emit classificationForResult(RESULT::Starch, res);
        } else if (res == "糖") {
            emit classificationForResult(RESULT::Sugar, res);
        } else if (res == "盐") {
            emit classificationForResult(RESULT::Salt, res);
        } else if (res == "小苏打") {
            emit classificationForResult(RESULT::SodiumBicarbonate, res);
        } else if (res == "洗衣粉") {
            emit classificationForResult(RESULT::WashingPowder, res);
        } else if (res == "橄榄油") {
            emit classificationForResult(RESULT::OliveOil, res);
        } else if (res == "水") {
            emit classificationForResult(RESULT::Water, res);
        } else if (res == "糖水") {
            emit classificationForResult(RESULT::TongSui, res);
        } else if (res == "芝麻油") {
            emit classificationForResult(RESULT::SesameOil, res);
        } else if (res == "葵花籽油") {
            emit classificationForResult(RESULT::SunflowerOil, res);
        } else if (res == "75酒精") {
            emit classificationForResult(RESULT::Alcohol75, res);
        } else if (res == "C2H4O2") {
            emit classificationForResult(RESULT::C2H4O2, res);
        } else if (res == "C2H6O") {
            emit classificationForResult(RESULT::C2H6O, res);
        } else if (res == "玉米油") {
            emit classificationForResult(RESULT::CornOil, res);
        } else if (res == "空瓶") {
            emit classificationForResult(RESULT::EmptyBottle, res);
        } else {
            emit classificationForResult(RESULT::Empty, res);
        }

        emit classificationForHistory(obj);
        cleanup();
    });

    connect(http, &MyHttp::httpError, this, [=](const QString &err) {
        qWarning() << "HTTP error:" << err;
        cleanup();
    });

    http->postJson(url, inputObj);
}

void ThreadWorker::onPlotSeparationStandard()
{
    MyHttp http;
    QJsonObject obj = http.get_sync("http://192.168.123.233:5015/standard");
    double current_min = std::numeric_limits<qreal>::max();
    double current_max = std::numeric_limits<qreal>::min();
    auto toPoints = [&](const QJsonArray &arr) {
        QList<QPointF> points;
        points.reserve(arr.size());
        current_min = std::numeric_limits<qreal>::max();
        current_max = std::numeric_limits<qreal>::min();
        for (int i = 0; i < arr.size(); ++i) {
            points.append(QPointF(i, arr[i].toDouble()));
            current_max = qMax(current_max, arr[i].toDouble());
            current_min = qMin(current_min, arr[i].toDouble());
        }
        return points;
    };
    emit sendSeparationSeries(toPoints(obj["sugar_curve"].toArray()),
                              "StandardSugar",
                              current_min,
                              current_max);
    emit sendSeparationSeries(toPoints(obj["salt_curve"].toArray()),
                              "StandardSalt",
                              current_min,
                              current_max);
    emit sendSeparationSeries(toPoints(obj["powder_curve"].toArray()),
                              "StandardPowder",
                              current_min,
                              current_max);
}

void ThreadWorker::sendSeparationRequest(const QVector<double> &v_voltage24)
{
    QJsonObject inputObj;
    QJsonArray signalArray;
    for (double val : v_voltage24)
        signalArray.append(val);
    inputObj["signal"] = signalArray;

    MyHttp *http = new MyHttp(this);
    QUrl url("http://192.168.123.233:5015/predict");

    auto cleanup = [http]() { http->deleteLater(); };

    connect(http, &MyHttp::jsonResponse, this, [=](const QJsonObject &obj) {
        QJsonObject predRatio = obj["pred_ratio"].toObject();
        double sugarRatio = predRatio["sugar"].toDouble();
        double saltRatio = predRatio["salt"].toDouble();
        double powderRatio = predRatio["powder"].toDouble();
        emit sendSeparationInfo(sugarRatio, saltRatio, powderRatio);
        double current_min = std::numeric_limits<qreal>::max();
        double current_max = std::numeric_limits<qreal>::min();
        auto toPoints = [&](const QJsonArray &arr) {
            QList<QPointF> points;
            points.reserve(arr.size());
            current_min = std::numeric_limits<qreal>::max();
            current_max = std::numeric_limits<qreal>::min();
            for (int i = 0; i < arr.size(); ++i) {
                points.append(QPointF(i, arr[i].toDouble()));
                current_max = qMax(current_max, arr[i].toDouble());
                current_min = qMin(current_min, arr[i].toDouble());
            }
            return points;
        };

        emit sendSeparationSeries(toPoints(obj["mix_curve"].toArray()),
                                  "Mix",
                                  current_min,
                                  current_max);
        emit sendSeparationSeries(toPoints(obj["sugar_curve"].toArray()),
                                  "Sugar",
                                  current_min,
                                  current_max);
        emit sendSeparationSeries(toPoints(obj["salt_curve"].toArray()),
                                  "Salt",
                                  current_min,
                                  current_max);
        emit sendSeparationSeries(toPoints(obj["powder_curve"].toArray()),
                                  "Powder",
                                  current_min,
                                  current_max);

        cleanup();
    });

    connect(http, &MyHttp::httpError, this, [=](const QString &err) {
        qWarning() << "HTTP error:" << err;
        cleanup();
    });

    http->postJson(url, inputObj);
}
