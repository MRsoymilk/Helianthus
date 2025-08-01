#include "formsetting.h"
#include "AutoUpdate/autoupdate.h"
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
    ui->vLay->addWidget(m_update);
}
