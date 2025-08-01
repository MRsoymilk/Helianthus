#include "autoupdate.h"
#include "g_define.h"
#include "myhttp.h"
#include "ui_autoupdate.h"
#include "version.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

AutoUpdate::AutoUpdate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AutoUpdate)
{
    ui->setupUi(this);
    m_http = new MyHttp;
}

AutoUpdate::~AutoUpdate()
{
    delete ui;
}

QJsonObject AutoUpdate::checkUpdate()
{
    m_url = SETTING_CONFIG_GET(CFG_GROUP_AUTOUPDATE,
                               CFG_AUTOUPDATE_URL,
                               "http://192.168.123.233:8020");
    QString url_json = QString("%1/%2").arg(m_url).arg("update.json");
    m_objUpdate = m_http->get_sync(url_json);
    return m_objUpdate;
}

bool AutoUpdate::isNewVersion()
{
    checkUpdate();

    QString currentVersion = APP_VERSION;
    QString newVersion = m_objUpdate["version"].toString();

    QStringList curList = currentVersion.split('.');
    QStringList newList = newVersion.split('.');

    int maxCount = qMax(curList.size(), newList.size());

    while (curList.size() < maxCount)
        curList << "0";
    while (newList.size() < maxCount)
        newList << "0";

    for (int i = 0; i < maxCount; ++i) {
        int curPart = curList[i].toInt();
        int newPart = newList[i].toInt();

        if (newPart > curPart)
            return true;
        else if (newPart < curPart)
            return false;
    }

    return false;
}

void AutoUpdate::on_btnUpdate_clicked()
{
    ui->progressBar->setVisible(true);
    QString to_downloaded = m_objUpdate["file"].toString();
    m_http->downloadBinary(
        ui->lineEditURL->text(),
        [=](QByteArray data) {
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QString fullPath = QDir(tempDir).filePath(to_downloaded);

            QFile file(fullPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
                ui->textBrowser->append("✅ 下载完成并保存成功：" + fullPath);

                QProcess::startDetached(fullPath);

            } else {
                ui->textBrowser->append("❌ 无法保存文件：" + fullPath);
            }
        },
        [=](QString err) { ui->textBrowser->append("❌ 下载失败: " + err); },
        [=](qint64 received, qint64 total) {
            if (total > 0) {
                int percent = static_cast<int>((double(received) / total) * 100);
                ui->progressBar->setValue(percent);
            }
        });
}

void AutoUpdate::showEvent(QShowEvent *event)
{
    QJsonObject res = checkUpdate();

    ui->progressBar->setVisible(false);

    ui->lineEditURL->setText(QString("%1/%2").arg(res["url"].toString(), res["file"].toString()));
    ui->lineEditTargetVersion->setText(res["version"].toString());
    ui->lineEditCurrentVersion->setText(APP_VERSION);
}
