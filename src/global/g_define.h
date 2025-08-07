#ifndef G_DEFINE_H
#define G_DEFINE_H

#include <QString>

enum class FORM { SERIAL = 0, OTO, RESULT, PLOT, HISTORY, SETTING };
enum class RESULT {
    Empty = 0,
    Starch,
    Sugar,
    Salt,
    NaHCO3,
    TongSui,
    OliveOil,
    SesameOil,
    SunflowerOil,
    Water,
    Alcohol75,
    C2H4O2,
    C2H6O,
    CornOil,
    EmptyBottle,
};

const QString CFG_GROUP_SERIAL = "Serial";
const QString CFG_SERIAL_DEBUG_PORT = "debug_port";
const QString CFG_SERIAL_PORT = "port";
const QString CFG_SERIAL_BAUD_RATE = "baud_rate";
const QString CFG_HISTORY_SEND = "history_send";
const QString CFG_GROUP_PROGRAM = "Program";
const QString CFG_PROGRAM_THEME = "theme";

const QString CFG_GROUP_FRAME = "Frame";
const QString CFG_FRAME_NAME = "name";
const QString CFG_FRAME_HEAD = "head";
const QString CFG_FRAME_FOOT = "foot";
const QString CFG_FRAME_LENGTH = "length";

const QString CFG_GROUP_AUTOUPDATE = "AutoUpdate";
const QString CFG_AUTOUPDATE_URL = "url";

const QString CFG_GROUP_HISTORY = "History";
const QString CFG_HISTORY_MULT = "mult";
const QString CFG_HISTORY_CMD = "cmd";

const QString CFG_GROUP_OTO = "OTO";
const QString CFG_OTO_INFO = "info";
const QString CFG_OTO_SPECTRUM = "spectrum";

const QString VAL_ENABLE = "enalbe";
const QString VAL_DISABLE = "disable";

#include <QDateTime>
#define TIMESTAMP_0() (QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
#define TIMESTAMP_1(format) (QDateTime::currentDateTime().toString(format))
#define TIMESTAMP(...) TIMESTAMP_MACRO(__VA_ARGS__, TIMESTAMP_1, TIMESTAMP_0)(__VA_ARGS__)

#include "mylog.h"
#define LOG_TRACE(...) MY_LOG.getLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) MY_LOG.getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) MY_LOG.getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) MY_LOG.getLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) MY_LOG.getLogger()->critical(__VA_ARGS__)
#define FORMAT_HEX(data) \
    ([](const QByteArray &d) { \
        QStringList hexList; \
        for (uchar byte : d) { \
            hexList << QString("0x%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper(); \
        } \
        return hexList.join(" "); \
    })(data)

#include "mysetting.h"
#define SETTING_CONFIG_GET(group, key, ...) \
    MY_SETTING.getValue(MySetting::SETTING::CONFIG, group, key, ##__VA_ARGS__)
#define SETTING_CONFIG_SET(group, key, value) \
    MY_SETTING.setValue(MySetting::SETTING::CONFIG, group, key, value)
#define SETTING_CONFIG_GROUPS() MY_SETTING.getGroups(MySetting::SETTING::CONFIG)
#define SETTING_CONFIG_SYNC() MY_SETTING.sync(MySetting::SETTING::CONFIG)

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QTimer>
#define SHOW_AUTO_CLOSE_MSGBOX(PARENT, TITLE, TEXT) \
    do { \
        QMessageBox *msgBox = new QMessageBox(PARENT); \
        msgBox->setWindowTitle(TITLE); \
        msgBox->setText(TEXT); \
        msgBox->setAttribute(Qt::WA_DeleteOnClose, false); \
        msgBox->show(); \
\
        QPropertyAnimation *fadeAnim = new QPropertyAnimation(msgBox, "windowOpacity", msgBox); \
        fadeAnim->setDuration(1000); \
        fadeAnim->setStartValue(1.0); \
        fadeAnim->setEndValue(0.0); \
        fadeAnim->setEasingCurve(QEasingCurve::InOutQuad); \
\
        QTimer::singleShot(1000, [msgBox, fadeAnim]() { \
            fadeAnim->start(); \
            QObject::connect(fadeAnim, &QPropertyAnimation::finished, msgBox, [msgBox]() { \
                msgBox->close(); \
                msgBox->deleteLater(); \
            }); \
        }); \
    } while (0)

const QString TITLE_INFO = "info";
const QString TITLE_WARNING = "warning";

#include "myprocess.h"
#define PROCESS(cmd, onSuccess, onError) \
    do { \
        auto *p = new MyProcess(); \
        p->setCallbacks(onSuccess, onError); \
        p->start(cmd); \
    } while (0)

#define PROCESS_ARGS(cmd, args, onSuccess, onError) \
    do { \
        auto *p = new MyProcess(); \
        p->setCallbacks((onSuccess), (onError)); \
        p->start((cmd), (args)); \
    } while (0)

#endif
