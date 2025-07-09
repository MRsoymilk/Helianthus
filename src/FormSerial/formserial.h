#ifndef FORMSERIAL_H
#define FORMSERIAL_H

#include <QSerialPort>
#include <QWidget>
#include "SerialData/serialdata.h"

namespace Ui {
class FormSerial;
}

class FormSerial : public QWidget
{
    Q_OBJECT
public:
    struct FRAME
    {
        QByteArray bit24;
    };
    struct FrameType
    {
        QString name;
        QByteArray header;
        QByteArray footer;
    };

    struct SERIAL
    {
        QString Description;
        QString Manufacturer;
        QString SerialNumber;
        QString SystemLocation;
        QList<qint32> StandardBaudRates;
    };

    struct INI_SERIAL
    {
        QString port;
        QString debug_port;
        QString baud_rate;
    };

public:
    explicit FormSerial(QWidget *parent = nullptr);
    ~FormSerial();

signals:
    void recvSerialData(const QByteArray &data);

private slots:
    void onReadyRead();
    void on_btnSerialSwitch_clicked();
    void on_btnSend_clicked();
    void on_tBtnData_clicked();
    void onSerialDataClose();

private:
    Ui::FormSerial *ui;
    void init();
    QList<FrameType> m_frameTypes = {};
    QMap<QString, SERIAL> m_mapSerial;
    bool openSerial();
    QSerialPort *m_serial = nullptr;
    INI_SERIAL m_ini;
    QByteArray m_buffer;
    FRAME frame;
    bool m_switch;
    void closeSerial();
    void send(const QString &text);
    SerialData *m_data;
    bool m_showData;
};

#endif // FORMSERIAL_H
