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
                        std::function<void(const QByteArray &, bool)> onData,
                        std::function<void(QString)> onError,
                        std::function<void(qint64, qint64)> onProgress);
    QJsonObject get_sync(const QString &url);
    void uploadFiles(const QUrl &url,
                     const QStringList &filePaths,
                     const QString &fieldName = "file",
                     const QMap<QString, QString> &extraFields = {},
                     std::function<void(const QJsonObject &)> onSuccess = nullptr,
                     std::function<void(const QString &)> onError = nullptr,
                     std::function<void(qint64, qint64)> onProgress = nullptr);
signals:
    void jsonResponse(const QJsonObject &obj);
    void httpError(const QString &error);

private:
    QNetworkAccessManager *m_manager;
};

#endif // MYHTTP_H
