#include "integration.h"
#include "g_define.h"
#include "ui_integration.h"

Integration::Integration(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Integration)
{
    ui->setupUi(this);
}

Integration::~Integration()
{
    delete ui;
}

int Integration::getTimes(const QString &hexString)
{
    QString start = "DD3C000622";
    QString end = "CDFF";

    int startIndex = hexString.indexOf(start);
    int endIndex = hexString.indexOf(end);

    if (startIndex == -1 || endIndex == -1 || endIndex <= startIndex + start.length()) {
        qWarning() << "Invalid format!";
        return 0;
    }

    int midStart = startIndex + start.length();
    int midLength = endIndex - midStart;
    QString midHex = hexString.mid(midStart, midLength);
    bool ok = false;
    int value = midHex.toInt(&ok, 16);
    if (!ok) {
        qWarning() << "Hex conversion failed!";
    }

    return value / 5;
}

QString Integration::generateHex(int times)
{
    int rawValue = times * 5;
    QString hex = QString("%1").arg(rawValue, 6, 16, QLatin1Char('0')).toUpper();
    QString prefix = "DD3C000622";
    QString suffix = "CDFF";
    return prefix + hex + suffix;
}

void Integration::on_btnConversion_clicked()
{
    QString edit_hex = ui->lineEditHex->text();
    QString edit_times = ui->lineEditTimes->text();
    if (edit_hex.isEmpty() && edit_times.isEmpty()) {
        SHOW_AUTO_CLOSE_MSGBOX(this, TITLE_WARNING, tr("Both hex and times cannot be empty!"));
        return;
    }
    if (edit_hex.isEmpty()) {
        ui->lineEditHex->setText(generateHex(edit_times.toInt()));
    } else if (edit_times.isEmpty()) {
        ui->lineEditTimes->setText(QString::number(getTimes(edit_hex)));
    }
}
