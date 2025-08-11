#include "myhttp.h"
#include <QDebug>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimer>

MyHttp::MyHttp(QObject *parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}

void MyHttp::postJson(const QUrl &url, const QJsonObject &json)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_manager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit httpError(reply->errorString());
        } else {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
            if (err.error != QJsonParseError::NoError) {
                emit httpError("JSON parse error: " + err.errorString());
            } else {
                emit jsonResponse(doc.object());
            }
        }
        reply->deleteLater();
    });
}

void MyHttp::downloadBinary(const QString &url,
                            std::function<void(const QByteArray &chunk, bool finished)> onData,
                            std::function<void(QString)> onError,
                            std::function<void(qint64, qint64)> onProgress)
{
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    QNetworkReply *reply = m_manager->get(request);

    // 下载进度
    if (onProgress) {
        connect(reply,
                &QNetworkReply::downloadProgress,
                reply,
                [onProgress](qint64 received, qint64 total) { onProgress(received, total); });
    }

    // 分块读取
    connect(reply, &QIODevice::readyRead, reply, [reply, onData]() {
        QByteArray chunk = reply->readAll(); // 读到当前已到达的所有数据
        if (!chunk.isEmpty()) {
            onData(chunk, false); // false = 还没完成
        }
    });

    // 下载完成
    connect(reply, &QNetworkReply::finished, reply, [reply, onData, onError]() {
        if (reply->error() != QNetworkReply::NoError) {
            onError(reply->errorString());
        } else {
            QByteArray lastChunk = reply->readAll(); // 可能还有没读的
            if (!lastChunk.isEmpty()) {
                onData(lastChunk, false);
            }
            onData(QByteArray(), true); // true = 已完成
        }
        reply->deleteLater();
    });

    // 错误
    connect(reply,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            reply,
            [onError](QNetworkReply::NetworkError) { onError("网络错误"); });
}

QJsonObject MyHttp::get_sync(const QString &url)
{
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    QNetworkReply *reply = m_manager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); // 等待请求完成

    QJsonObject result;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            result = doc.object();
        } else {
            qWarning() << "[MyHttp] JSON parse error:" << err.errorString();
        }
    } else {
        qWarning() << "[MyHttp] GET Error:" << reply->errorString();
    }

    reply->deleteLater();
    return result;
}
