#include "serialframe.h"
#include "g_define.h"
#include "ui_serialframe.h"

SerialFrame::SerialFrame(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SerialFrame)
{
    ui->setupUi(this);
}

SerialFrame::~SerialFrame()
{
    delete ui;
}

void SerialFrame::showEvent(QShowEvent *event)
{
    ui->lineEditHead->setText(SETTING_CONFIG_GET(CFG_GROUP_FRAME, CFG_FRAME_HEAD));
    ui->lineEditFoot->setText(SETTING_CONFIG_GET(CFG_GROUP_FRAME, CFG_FRAME_FOOT));
    ui->lineEditLength->setText(SETTING_CONFIG_GET(CFG_GROUP_FRAME, CFG_FRAME_LENGTH));
}

void SerialFrame::on_btnModify_clicked()
{
    SETTING_CONFIG_SET(CFG_GROUP_FRAME, CFG_FRAME_HEAD, ui->lineEditHead->text());
    SETTING_CONFIG_SET(CFG_GROUP_FRAME, CFG_FRAME_FOOT, ui->lineEditFoot->text());
    SETTING_CONFIG_SET(CFG_GROUP_FRAME, CFG_FRAME_LENGTH, ui->lineEditLength->text());
}
