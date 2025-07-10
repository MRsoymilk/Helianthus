#include "myhttp.h"
#include <QDebug>
#include <QJsonDocument>
#include <QNetworkRequest>

MyHttp::MyHttp(QObject *parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &MyHttp::onReplyFinished);
}

void MyHttp::postJson(const QUrl &url, const QJsonObject &json)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    m_manager->post(request, jsonData);
}

void MyHttp::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit httpError(reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        emit httpError("JSON parse error: " + err.errorString());
        reply->deleteLater();
        return;
    }

    emit jsonResponse(doc.object());
    reply->deleteLater();
}
