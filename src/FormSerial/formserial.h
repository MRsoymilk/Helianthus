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
        int length;
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

    void on_tBtnRefresh_clicked();

    void on_tBtnIntegration_clicked();

private:
    void init();
    void initMultipe();
    bool openSerial();
    void closeSerial();
    void send(const QString &text);

private:
    Ui::FormSerial *ui;
    QList<FrameType> m_frameTypes = {};
    QMap<QString, SERIAL> m_mapSerial;
    QSerialPort *m_serial = nullptr;
    INI_SERIAL m_ini;
    QByteArray m_buffer;
    FRAME frame;
    bool m_switch;
    SerialData *m_data;
    bool m_showData;
    long long m_recv_count;
    void handleFrame(const QString &type, const QByteArray &data);
    void refreshSerialPorts();
    QStringList m_lastPortList;
};

#endif // FORMSERIAL_H
