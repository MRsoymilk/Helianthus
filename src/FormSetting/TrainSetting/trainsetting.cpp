#include "trainsetting.h"
#include "g_define.h"
#include "ui_trainsetting.h"

TrainSetting::TrainSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TrainSetting)
{
    ui->setupUi(this);
    init();
}

TrainSetting::~TrainSetting()
{
    delete ui;
}

void TrainSetting::init()
{
    QString url = SETTING_CONFIG_GET(CFG_GROUP_TRAIN, CFG_URL);
    ui->lineEditURL->setText(url);
}

void TrainSetting::on_btnModify_clicked()
{
    SETTING_CONFIG_SET(CFG_GROUP_TRAIN, CFG_URL, ui->lineEditURL->text());
}
