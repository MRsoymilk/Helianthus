#include "serialdata.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardItemModel>
#include <QTextStream>
#include "ui_serialdata.h"

SerialData::SerialData(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SerialData)
    , m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    m_model->setHorizontalHeaderLabels({"timestamp", "data", "size"});
    ui->tableView->setModel(m_model);

    ui->spinBoxShowLimit->setMinimum(1);
    ui->spinBoxShowLimit->setMaximum(10000);
    ui->spinBoxShowLimit->setValue(100);
}

SerialData::~SerialData()
{
    delete ui;
}

void SerialData::onSerialDataReceive(const QByteArray &data)
{
    if (this->isVisible()) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString dataHex = data.toHex(' ').toUpper();
        int size = data.size();

        QList<QStandardItem *> row;
        row << new QStandardItem(timestamp);
        row << new QStandardItem(dataHex);
        row << new QStandardItem(QString::number(size));
        m_model->appendRow(row);

        int limit = ui->spinBoxShowLimit->value();
        while (m_model->rowCount() > limit) {
            m_model->removeRow(0);
        }

        QDir dir("data");
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QString fileName = QString("data/%1.csv").arg("raw");
        QFile file(fileName);

        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << timestamp << "," << dataHex << "," << size << "\n";
            file.close();
        }
    }
}

void SerialData::closeEvent(QCloseEvent *event)
{
    emit serialDataClose();
    QWidget::closeEvent(event);
}
