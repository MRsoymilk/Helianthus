#ifndef MYHTTP_H
#define MYHTTP_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class MyHttp : public QObject
{
    Q_OBJECT
public:
    explicit MyHttp(QObject *parent = nullptr);
    void postJson(const QUrl &url, const QJsonObject &json);
    void downloadBinary(const QString &url,
                        std::function<void(QByteArray)> onSuccess,
                        std::function<void(QString)> onError,
                        std::function<void(qint64, qint64)> onProgress);
    QJsonObject get_sync(const QString &url);

signals:
    void jsonResponse(const QJsonObject &obj);
    void httpError(const QString &error);

private:
    QNetworkAccessManager *m_manager;
};

#endif // MYHTTP_H
