#include "formoto.h"
#include "ui_formoto.h"

#include <QInputDialog>

#include "g_define.h"
#include "myhttp.h"

FormOTO::FormOTO(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormOTO)
{
    ui->setupUi(this);
    init();
}

FormOTO::~FormOTO()
{
    delete ui;
}

void FormOTO::onOtoRequestRaw(const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    QString jsonStr = doc.toJson(QJsonDocument::Indented);
    ui->textBrowserData->setText(jsonStr);
}

void FormOTO::init()
{
    m_switch = false;
    QString url_info = SETTING_CONFIG_GET(CFG_GROUP_OTO,
                                          CFG_OTO_INFO,
                                          "http://192.168.123.233:9000/info");
    QString url_spectrum = SETTING_CONFIG_GET(CFG_GROUP_OTO,
                                              CFG_OTO_SPECTRUM,
                                              "http://192.168.123.233:9000/spectrum");
    ui->lineEditURLInfo->setText(url_info);
    ui->lineEditURLSpectrum->setText(url_spectrum);
    ui->btnSendSpectrum->setCheckable(true);
    ui->btnSendSpectrum->setText("To Open");
}

void FormOTO::on_btnSendSpectrum_clicked()
{
    m_switch = !m_switch;
    ui->btnSendSpectrum->setChecked(m_switch);

    if (m_switch) {
        SETTING_CONFIG_SET(CFG_GROUP_OTO, CFG_OTO_SPECTRUM, ui->lineEditURLSpectrum->text());
        ui->btnSendSpectrum->setText(tr("To Close"));
        emit otoRequest(true, ui->lineEditURLSpectrum->text());
        emit otoCallParams();
    } else {
        ui->btnSendSpectrum->setText(tr("To Open"));
        emit otoRequest(false, "");
    }
}

void FormOTO::on_btnSendInfo_clicked()
{
    SETTING_CONFIG_SET(CFG_GROUP_OTO, CFG_OTO_INFO, ui->lineEditURLInfo->text());
    MyHttp *http = new MyHttp(this);
    QJsonObject obj = http->get_sync(ui->lineEditURLInfo->text());
    QString info = QString("info: %1\nstatus: %2")
                       .arg(obj["model_name"].toString())
                       .arg(obj["status"].toString());
    ui->textBrowserData->setText(info);
}
