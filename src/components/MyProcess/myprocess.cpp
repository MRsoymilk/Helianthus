#include "myprocess.h"
#include "g_define.h"

MyProcess::MyProcess(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    connect(m_process,
            &QProcess::readyReadStandardOutput,
            this,
            &MyProcess::onReadyReadStandardOutput);
    connect(m_process,
            &QProcess::readyReadStandardError,
            this,
            &MyProcess::onReadyReadStandardError);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &MyProcess::onFinished);
}

MyProcess::~MyProcess()
{
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
    }
}

void MyProcess::setCallbacks(Callback successCallback, Callback errorCallback)
{
    m_cbSuccess = successCallback;
    m_cbError = errorCallback;
}

QString MyProcess::runSync(const QString &program, const QStringList &arguments)
{
    QProcess proc;
    proc.start(program, arguments);
    proc.waitForFinished();

    QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    QString error = QString::fromLocal8Bit(proc.readAllStandardError());

    if (!error.isEmpty()) {
        LOG_WARN("Process error: {}", error);
    }

    return output;
}

void MyProcess::start(const QString &program, const QStringList &arguments)
{
    m_process->start(program, arguments);
}

void MyProcess::onReadyReadStandardOutput()
{
    QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    emit readyReadStandardOutput(output);
    if (m_cbSuccess)
        m_cbSuccess(output);
}

void MyProcess::onReadyReadStandardError()
{
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    emit readyReadStandardError(error);
    if (m_cbError)
        m_cbError(error);
}

void MyProcess::onFinished(int exitCode)
{
    emit finished(exitCode);
}
