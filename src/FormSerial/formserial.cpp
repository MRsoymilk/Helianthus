#include "formserial.h"
#include "g_define.h"
#include "ui_formserial.h"

#include <QFile>
#include <QSerialPortInfo>

FormSerial::FormSerial(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormSerial)
{
    ui->setupUi(this);
    init();
}

FormSerial::~FormSerial()
{
    delete ui;
}

void FormSerial::refreshSerialPorts()
{
    QStringList currentPorts;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const auto &port : ports) {
        currentPorts << port.portName();
    }

    if (currentPorts != m_lastPortList) {
        LOG_INFO("Serial ports changed: {}", currentPorts.join(", ").toStdString());
        m_lastPortList = currentPorts;

        ui->comboBoxPort->clear();
        ui->comboBoxPort->addItems(currentPorts);

        m_mapSerial.clear();
        for (const auto &port : ports) {
            SERIAL serial;
            serial.SerialNumber = port.serialNumber();
            serial.Description = port.description();
            serial.Manufacturer = port.manufacturer();
            serial.StandardBaudRates = port.standardBaudRates();
            serial.SystemLocation = port.systemLocation();
            m_mapSerial.insert(port.portName(), serial);
        }
    }
}

void FormSerial::init()
{
    // init port
    QList<QSerialPortInfo> list_port = QSerialPortInfo::availablePorts();
    if (list_port.isEmpty()) {
        LOG_WARN("No available serial port found!");
        return;
    }
    QStringList port_names;
    for (const auto &port : list_port) {
        SERIAL serial;
        serial.SerialNumber = port.serialNumber();
        serial.Description = port.description();
        serial.Manufacturer = port.manufacturer();
        serial.StandardBaudRates = port.standardBaudRates();
        serial.SystemLocation = port.systemLocation();
        m_mapSerial.insert(port.portName(), serial);
        port_names.push_back(port.portName());
    }

    m_switch = false;
    ui->btnSerialSwitch->setCheckable(true);
    ui->btnSerialSwitch->setText("To Open");

    ui->comboBoxPort->addItems(port_names);
    QList<qint32> baudRates = m_mapSerial[port_names.at(0)].StandardBaudRates;
    for (qint32 rate : baudRates) {
        ui->comboBoxBaudRate->addItem(QString::number(rate));
    }

    ui->comboBoxDataBit->addItems({"8", "7", "6", "5"});
    ui->comboBoxCheckBit->addItems({"None", "Even", "Mark", "Odd"});
    ui->comboBoxStopBit->addItems({"1", "1.5", "2"});

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [=]() {
        if (m_serial && m_serial->isOpen()) {
            QMetaObject::invokeMethod(m_serial, "close", Qt::QueuedConnection);
        }
    });

    m_frameTypes = {
        {"curve_24bit", QByteArray::fromHex("DE3A096631"), QByteArray::fromHex("CEFF"), 1990},
    };

    QString history_send = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_HISTORY_SEND, "");
    ui->textEditSend->setText(history_send);
    m_ini.debug_port = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_DEBUG_PORT, "");
    m_ini.port = SETTING_CONFIG_GET(CFG_GROUP_SERIAL, CFG_SERIAL_PORT, port_names.at(0));
    m_ini.baud_rate = SETTING_CONFIG_GET(CFG_GROUP_SERIAL,
                                         CFG_SERIAL_BAUD_RATE,
                                         QString::number(
                                             m_mapSerial[port_names.at(0)].StandardBaudRates.at(0)));
#ifdef QT_DEBUG
    ui->comboBoxPort->addItem(m_ini.debug_port);
    ui->comboBoxPort->setCurrentText(m_ini.debug_port);
#else
    ui->comboBoxPort->setCurrentText(m_ini.port);
#endif
    ui->comboBoxBaudRate->setCurrentText(m_ini.baud_rate);

    m_data = new SerialData;
    m_showData = false;
    ui->tBtnData->setCheckable(true);
    ui->tBtnData->setChecked(m_showData);
    connect(m_data, &SerialData::serialDataClose, this, &FormSerial::onSerialDataClose);
    connect(this, &FormSerial::recvSerialData, m_data, &SerialData::onSerialDataReceive);
}

void FormSerial::send(const QString &text)
{
    LOG_INFO("serial send: {}", text);
    if (!(m_serial && m_serial->isOpen())) {
        SHOW_AUTO_CLOSE_MSGBOX(this, "warning", "serial not open!");
        LOG_ERROR("Serial not open!");
        return;
    }

    if (text.isEmpty()) {
        LOG_WARN("Send txt is empty.");
        return;
    }

    QByteArray data;
    QString to_show;
    QString cleaned = text;
    cleaned.remove(QRegularExpression("[^0-9A-Fa-f\\s]"));

    QStringList byteStrings;
    if (cleaned.contains(QRegularExpression("\\s+"))) {
        byteStrings = cleaned.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    } else {
        for (int i = 0; i + 1 < cleaned.length(); i += 2) {
            byteStrings << cleaned.mid(i, 2);
        }
    }

    for (const QString &byteStr : byteStrings) {
        bool ok;
        int byte = byteStr.toInt(&ok, 16);
        if (ok) {
            data.append(static_cast<char>(byte));
        } else {
            LOG_WARN("illegal hex: {}", byteStr);
        }
    }
    LOG_INFO("send (hex): {}", data);
    to_show = data;
    auto res = m_serial->write(data);
    if (res == -1) {
        LOG_WARN("Write failed: {}", m_serial->errorString());
    }
}

void FormSerial::handleFrame(const QString &type, const QByteArray &data)
{
    if (type == "curve_24bit") {
        emit recvSerialData(data);
    }
}

void FormSerial::onReadyRead()
{
    QByteArray data = m_serial->readAll();
    m_recv_count += data.size();
    ui->labelRecvCount->setText(QString("recv: %1").arg(m_recv_count));

    QString to_show = data;
    to_show.clear();
    for (int i = 0; i < data.length(); ++i) {
        to_show.append(QString("%1 ").arg((unsigned char) data[i], 2, 16, QChar('0')).toUpper());
    }
    ui->textBrowserRecv->append("[RX] " + to_show);
    m_buffer.append(data);

    while (true) {
        int firstHeaderIdx = -1;
        FrameType current_frame;

        // 查找所有已知帧头
        for (const auto &type : m_frameTypes) {
            int idx = m_buffer.indexOf(type.header);
            if (idx != -1 && (firstHeaderIdx == -1 || idx < firstHeaderIdx)) {
                firstHeaderIdx = idx;
                current_frame = type;
            }
        }

        // 没有帧头，清理或等待
        if (firstHeaderIdx == -1) {
            if (m_buffer.size() > 10 * 1024) {
                LOG_WARN("Buffer overflow, clearing");
                m_buffer.clear();
            }
            break;
        }

        // 丢弃无效数据
        if (firstHeaderIdx > 0) {
            LOG_WARN("Dropping invalid data before header: {} bytes", firstHeaderIdx);
            m_buffer.remove(0, firstHeaderIdx);
        }

        // 长度固定帧
        if (m_buffer.size() < current_frame.length)
            break;

        QByteArray frame_candidate = m_buffer.left(current_frame.length);
        if (!frame_candidate.endsWith(current_frame.footer)) {
            LOG_WARN("Invalid footer (fixed length), removing header only");
            m_buffer.remove(0, current_frame.header.size());
            continue;
        }

        LOG_INFO("Fixed-length frame matched: {}", current_frame.name.toStdString());
        handleFrame(current_frame.name, frame_candidate);
        m_buffer.remove(0, current_frame.length);
    }
}

bool FormSerial::openSerial()
{
    LOG_INFO("open serial");
    m_serial = new QSerialPort(this);
    QString port_name = ui->comboBoxPort->currentText();
    m_serial->setPortName(port_name);
    int baud_rate = ui->comboBoxBaudRate->currentText().toInt();
    m_serial->setBaudRate(baud_rate);
    int data_bits = ui->comboBoxDataBit->currentText().toInt();
    m_serial->setDataBits(static_cast<QSerialPort::DataBits>(data_bits));

    QString check = ui->comboBoxCheckBit->currentText();
    if (check == "None")
        m_serial->setParity(QSerialPort::NoParity);
    else if (check == "Even")
        m_serial->setParity(QSerialPort::EvenParity);
    else if (check == "Odd")
        m_serial->setParity(QSerialPort::OddParity);
    else if (check == "Mark")
        m_serial->setParity(QSerialPort::MarkParity);

    QString stop_bits = ui->comboBoxStopBit->currentText();
    if (stop_bits == "1")
        m_serial->setStopBits(QSerialPort::OneStop);
    else if (stop_bits == "1.5")
        m_serial->setStopBits(QSerialPort::OneAndHalfStop);
    else if (stop_bits == "2")
        m_serial->setStopBits(QSerialPort::TwoStop);

    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        QString msg = "Failed to open serial port: " + m_serial->errorString();
        LOG_WARN(msg);
        delete m_serial;
        m_serial = nullptr;
        m_switch = false;
        SHOW_AUTO_CLOSE_MSGBOX(this, tr("Warning"), msg);
        return false;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &FormSerial::onReadyRead);

    SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_PORT, port_name);
    SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_SERIAL_BAUD_RATE, QString::number(baud_rate));
    return true;
}

void FormSerial::on_btnSerialSwitch_clicked()
{
    m_switch = !m_switch;
    if (m_switch) {
        // open serial
        openSerial();
    } else {
        // close serial
        closeSerial();
    }
    if (m_switch) {
        ui->btnSerialSwitch->setChecked(true);
        ui->btnSerialSwitch->setText(tr("To Close"));
    } else {
        ui->btnSerialSwitch->setChecked(false);
        ui->btnSerialSwitch->setText(tr("To Open"));
    }
}

void FormSerial::closeSerial()
{
    LOG_INFO("close serial");
    if (m_serial && m_serial->isOpen()) {
        m_serial->close();
    }
    delete m_serial;
    m_serial = nullptr;
}

void FormSerial::on_btnSend_clicked()
{
    QString text = ui->textEditSend->toPlainText().trimmed();
    LOG_INFO("serial send: {}", text);
    send(text);
    SETTING_CONFIG_SET(CFG_GROUP_SERIAL, CFG_HISTORY_SEND, text);
}

void FormSerial::onSerialDataClose()
{
    m_showData = false;
    ui->tBtnData->setChecked(false);
}

void FormSerial::on_tBtnData_clicked()
{
    m_showData = !m_showData;
    if (m_showData) {
        m_data->show();
    } else {
        m_data->close();
    }
}

void FormSerial::on_tBtnRefresh_clicked()
{
    refreshSerialPorts();
}
