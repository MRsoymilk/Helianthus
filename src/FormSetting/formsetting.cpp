#include "formsetting.h"
#include "AutoUpdate/autoupdate.h"
#include "ClassifySetting/classifysetting.h"
#include "SeparationSetting/separationsetting.h"
#include "SerialFrame/serialframe.h"
#include "TrainSetting/trainsetting.h"
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
    m_train = new TrainSetting;
    m_classify = new ClassifySetting;
    m_separation = new SeparationSetting;
    ui->vLay->addWidget(m_update);
    ui->vLay->addWidget(m_frame);
    ui->vLay->addWidget(m_train);
    ui->vLay->addWidget(m_classify);
    ui->vLay->addWidget(m_separation);
}
