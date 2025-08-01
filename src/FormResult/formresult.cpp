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
    ui->widget->load(QUrl("qrc:/res/page/Empty.html"));
}

void FormResult::showResult(RESULT result)
{
    if (result == lastResult) {
        return;
    }
    lastResult = result;
    switch (result) {
    case RESULT::Water:
        ui->widget->load(QUrl("qrc:/res/page/Water.html"));
        break;
    case RESULT::Alcohol75:
        ui->widget->load(QUrl("qrc:/res/page/Alcoholic_beverage.html"));
        break;
    case RESULT::C2H4O2:
        ui->widget->load(QUrl("qrc:/res/page/C2H4O2.html"));
        break;
    case RESULT::C2H6O:
        ui->widget->load(QUrl("qrc:/res/page/C2H6O.html"));
        break;
    case RESULT::CornOil:
        ui->widget->load(QUrl("qrc:/res/page/Corn_oil.html"));
        break;
    case RESULT::EmptyBottle:
        ui->widget->load(QUrl("qrc:/res/page/Bottle.html"));
        break;
    case RESULT::Empty:
        ui->widget->load(QUrl("qrc:/res/page/Empty.html"));
        break;
    case RESULT::Starch:
        ui->widget->load(QUrl("qrc:/res/page/Starch.html"));
        break;
    case RESULT::Sugar:
        ui->widget->load(QUrl("qrc:/res/page/Sugar.html"));
        break;
    case RESULT::Salt:
        ui->widget->load(QUrl("qrc:/res/page/Salt.html"));
        break;
    case RESULT::NaHCO3:
        ui->widget->load(QUrl("qrc:/res/page/Sodium_bicarbonate.html"));
        break;
    case RESULT::TongSui:
        ui->widget->load(QUrl("qrc:/res/page/Tong_sui.html"));
        break;
    case RESULT::OliveOil:
        ui->widget->load(QUrl("qrc:/res/page/Olive_oil.html"));
        break;
    case RESULT::SesameOil:
        ui->widget->load(QUrl("qrc:/res/page/Sesame_oil.html"));
        break;
    case RESULT::SunflowerOil:
        ui->widget->load(QUrl("qrc:/res/page/Sunflower_oil.html"));
        break;
    }
}
