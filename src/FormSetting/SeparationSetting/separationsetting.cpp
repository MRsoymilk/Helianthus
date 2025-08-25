#include "separationsetting.h"
#include "g_define.h"
#include "ui_separationsetting.h"

SeparationSetting::SeparationSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SeparationSetting)
{
    ui->setupUi(this);
    init();
}

SeparationSetting::~SeparationSetting()
{
    delete ui;
}

void SeparationSetting::init()
{
    QString url = SETTING_CONFIG_GET(CFG_GROUP_SEPARATION, CFG_URL);
    ui->lineEditURL->setText(url);
}

void SeparationSetting::on_btnModify_clicked()
{
    SETTING_CONFIG_SET(CFG_GROUP_SEPARATION, CFG_URL, ui->lineEditURL->text());
}
