#include "classifysetting.h"
#include "g_define.h"
#include "ui_classifysetting.h"

ClassifySetting::ClassifySetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ClassifySetting)
{
    ui->setupUi(this);
}

ClassifySetting::~ClassifySetting()
{
    delete ui;
}

void ClassifySetting::on_btnModify_clicked()
{
    SETTING_CONFIG_SET(CFG_GROUP_CLASSIFY, CFG_URL, ui->lineEditURL->text());
}

void ClassifySetting::init()
{
    QString url = SETTING_CONFIG_GET(CFG_GROUP_CLASSIFY, CFG_URL);
    ui->lineEditURL->setText(url);
}
