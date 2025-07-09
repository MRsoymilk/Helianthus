#ifndef MYPROCESS_H
#define MYPROCESS_H

#include <QObject>
#include <QProcess>
#include <functional>

class MyProcess : public QObject
{
    Q_OBJECT
public:
    using Callback = std::function<void(const QString &)>;

    explicit MyProcess(QObject *parent = nullptr);
    ~MyProcess();

    void start(const QString &program, const QStringList &arguments = {});
    void setCallbacks(Callback successCallback, Callback errorCallback);
    QString runSync(const QString &program, const QStringList &arguments);

signals:
    void finished(int exitCode);
    void readyReadStandardOutput(const QString &text);
    void readyReadStandardError(const QString &text);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onFinished(int exitCode);

private:
    QProcess *m_process = nullptr;
    Callback m_cbSuccess;
    Callback m_cbError;
};

#endif // MYPROCESS_H
