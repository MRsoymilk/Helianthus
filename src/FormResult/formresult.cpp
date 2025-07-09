#include "formresult.h"
#include "ui_formresult.h"

FormResult::FormResult(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormResult)
{
    ui->setupUi(this);
    init();
}

FormResult::~FormResult()
{
    delete ui;
}

void FormResult::init()
{
    ui->widget->page()->runJavaScript(R"(
    document.querySelectorAll('a').forEach(el => {
        el.addEventListener('click', e => e.preventDefault());
    });
)");
    ui->widget->load(QUrl("qrc:/res/page/空.html"));
}

void FormResult::showResult(RESULT result)
{
    if (result == lastResult) {
        return;
    }
    lastResult = result;
    switch (result) {
    case RESULT::Water:
        ui->widget->load(QUrl("qrc:/res/page/水.html"));
        break;
    case RESULT::Alcohol75:
        ui->widget->load(QUrl("qrc:/res/page/75酒精.html"));
        break;
    case RESULT::C2H4O2:
        ui->widget->load(QUrl("qrc:/res/page/C2H4O2.html"));
        break;
    case RESULT::C2H6O:
        ui->widget->load(QUrl("qrc:/res/page/C2H6O.html"));
        break;
    case RESULT::CornOil:
        ui->widget->load(QUrl("qrc:/res/page/玉米油.html"));
        break;
    case RESULT::EmptyBottle:
        ui->widget->load(QUrl("qrc:/res/page/空瓶.html"));
        break;
    case RESULT::Empty:
        ui->widget->load(QUrl("qrc:/res/page/空.html"));
        break;
    case RESULT::Starch:
        ui->widget->load(QUrl("qrc:/res/page/淀粉.html"));
        break;
    case RESULT::Sugar:
        ui->widget->load(QUrl("qrc:/res/page/糖.html"));
        break;
    case RESULT::Salt:
        ui->widget->load(QUrl("qrc:/res/page/食盐.html"));
        break;
    case RESULT::NaHCO3:
        ui->widget->load(QUrl("qrc:/res/page/碳酸氢钠.html"));
        break;
    case RESULT::TongSui:
        ui->widget->load(QUrl("qrc:/res/page/糖水.html"));
        break;
    case RESULT::OliveOil:
        ui->widget->load(QUrl("qrc:/res/page/橄榄油.html"));
        break;
    case RESULT::SesameOil:
        ui->widget->load(QUrl("qrc:/res/page/芝麻油.html"));
        break;
    case RESULT::SunflowerOil:
        ui->widget->load(QUrl("qrc:/res/page/葵花籽油.html"));
        break;
    }
}
