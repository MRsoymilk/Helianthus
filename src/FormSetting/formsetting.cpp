#include "formsetting.h"
#include "AutoUpdate/autoupdate.h"
#include "SerialFrame/serialframe.h"
#include "ui_formsetting.h"

FormSetting::FormSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormSetting)
{
    ui->setupUi(this);
    init();
}

FormSetting::~FormSetting()
{
    delete ui;
}

void FormSetting::init()
{
    m_update = new AutoUpdate;
    m_frame = new SerialFrame;
    ui->vLay->addWidget(m_update);
    ui->vLay->addWidget(m_frame);
}
