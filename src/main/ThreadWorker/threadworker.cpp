#include "threadworker.h"
#include "g_define.h"

#include <QJsonArray>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "myhttp.h"
#include <cmath>
#include <complex>
#include <fftw3.h>

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
    m_self_train_record = false;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ThreadWorker::otoRequest);
    m_url_separation = SETTING_CONFIG_GET(CFG_GROUP_SEPARATION,
                                          CFG_URL,
                                          "http://192.168.123.233:5015");
    m_url_classify = SETTING_CONFIG_GET(CFG_GROUP_CLASSIFY, CFG_URL, "http://192.168.123.233:5010");
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

    if (m_do_derivative) {
    }
    if (m_do_fourier) {
    }
    emit dataForTableReady(v_voltage24, raw24);
    emit dataForPlotReady(out24, xMin, xMax, yMin, yMax);

    if (m_self_train_record) {
        emit dataForSelfTrain(out24, xMin, xMax, yMin, yMax);
    }

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

    if (m_do_derivative) {
        derivativeData(v_voltage24);
    }
    if (m_do_fourier) {
        yMin = std::numeric_limits<double>::max();
        yMax = std::numeric_limits<double>::lowest();
        v_voltage24 = fourierData(v_voltage24);
        for (int i = 0; i < out24.size(); ++i) {
            out24[i].setY(v_voltage24[i]);
            yMin = std::min(yMin, out24[i].y());
            yMax = std::max(yMax, out24[i].y());
        }
    }

    emit dataForTableReady(v_voltage24, raw24);
    emit dataForPlotReady(out24, xMin, xMax, yMin, yMax);

    if (m_self_train_record) {
        emit dataForSelfTrain(out24, xMin, xMax, yMin, yMax);
    }

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

void ThreadWorker::derivativeData(const QVector<double> &v)
{
    int N = v.size();
    if (N < 2) {
        return;
    }

    QVector<double> deriv(N);

    // 边界处理：前向差分 & 后向差分
    deriv[0] = v[1] - v[0];
    deriv[N - 1] = v[N - 1] - v[N - 2];

    // 中心差分
    for (int i = 1; i < N - 1; i++) {
        deriv[i] = (v[i + 1] - v[i - 1]) / 2.0;
    }
    emit derivativeReady(deriv);
}

QVector<double> ThreadWorker::fourierData(const QVector<double> &v, double fs, double cutoff_ratio)
{
    int N = v.size();
    if (N == 0)
        return {};

    // 分配 FFTW 输入输出
    double *in = (double *) fftw_malloc(sizeof(double) * N);
    fftw_complex *out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));

    for (int i = 0; i < N; ++i)
        in[i] = v[i];

    // 正向 FFT
    fftw_plan p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(p);

    // 频率坐标
    QVector<double> freq(N / 2 + 1);
    for (int i = 0; i <= N / 2; ++i)
        freq[i] = i * fs / N;

    // 幅值谱
    QVector<double> magnitude(N / 2 + 1);
    for (int i = 0; i <= N / 2; ++i) {
        double real = out[i][0];
        double imag = out[i][1];
        magnitude[i] = std::sqrt(real * real + imag * imag);
    }

    // 低通滤波
    int cutoff = int(cutoff_ratio * N);
    for (int i = cutoff; i <= N / 2; ++i) {
        out[i][0] = 0;
        out[i][1] = 0;
    }

    // 逆变换
    QVector<double> smoothed(N);
    fftw_plan backward = fftw_plan_dft_c2r_1d(N, out, smoothed.data(), FFTW_ESTIMATE);
    fftw_execute(backward);

    // 归一化
    for (int i = 0; i < N; ++i)
        smoothed[i] /= N;

    // 释放资源
    fftw_destroy_plan(p);
    fftw_destroy_plan(backward);
    fftw_free(in);
    fftw_free(out);

    // 发信号
    emit fourierSpectrumReady(freq, magnitude);

    return smoothed;
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
    QUrl url(QString("%1/knn_predict").arg(m_url_classify));

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

void ThreadWorker::toPoints(const QJsonArray &arr, QString name)
{
    QList<QPointF> points;
    points.reserve(arr.size());
    qreal local_min = std::numeric_limits<qreal>::max();
    qreal local_max = std::numeric_limits<qreal>::min();

    for (int i = 0; i < arr.size(); ++i) {
        double val = arr[i].toDouble();
        points.append(QPointF(i, val));
        local_max = qMax(local_max, val);
        local_min = qMin(local_min, val);
    }

    // 阈值判断：如果曲线接近全 0，就直接跳过
    if (qFuzzyIsNull(local_max) || qAbs(local_max) < 1e-6) {
        return; // 不发送
    }

    emit sendSeparationSeries(points, name, local_min, local_max);
}

void ThreadWorker::onPlotSeparationStandard()
{
    MyHttp http;
    QJsonObject obj = http.get_sync(QString("%1/standard").arg(m_url_separation));

    for (auto key : obj.keys()) {
        toPoints(obj[key].toArray(), key + "_base");
    }
}

void ThreadWorker::onSelfTrainRecord(bool isDo)
{
    m_self_train_record = isDo;
}

void ThreadWorker::doDerivative(bool isDo)
{
    m_do_derivative = isDo;
}

void ThreadWorker::doFourier(bool isDo)
{
    m_do_fourier = isDo;
}

void ThreadWorker::sendSeparationRequest(const QVector<double> &v_voltage24)
{
    QJsonObject inputObj;
    QJsonArray signalArray;
    for (double val : v_voltage24)
        signalArray.append(val);
    inputObj["signal"] = signalArray;

    MyHttp *http = new MyHttp(this);
    QUrl url(QString("%1/predict").arg(m_url_separation));

    auto cleanup = [http]() { http->deleteLater(); };

    connect(http, &MyHttp::jsonResponse, this, [=](const QJsonObject &obj) {
        QJsonObject predRatio = obj["pred_ratio"].toObject();

        QMap<QString, double> ratios;
        for (const auto key : predRatio.keys()) {
            ratios[key] = predRatio[key].toDouble();
        }
        emit sendSeparationInfo(ratios);

        QJsonObject objCurves = obj["separated_curves"].toObject();
        for (const auto key : objCurves.keys()) {
            toPoints(objCurves[key].toArray(), key);
        }
        toPoints(obj["mix_curve"].toArray(), "mix_curve");

        cleanup();
    });

    connect(http, &MyHttp::httpError, this, [=](const QString &err) {
        qWarning() << "HTTP error:" << err;
        cleanup();
    });

    http->postJson(url, inputObj);
}
