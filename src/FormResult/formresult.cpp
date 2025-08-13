#include "formresult.h"
#include "ui_formresult.h"

#include <QDir>

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

static QString resultToLocalFileUrl(RESULT result)
{
    static const QMap<RESULT, QString> resultMap = {
        {RESULT::Water, "Water.html"},
        {RESULT::Alcohol75, "Alcoholic_beverage.html"},
        {RESULT::C2H4O2, "C2H4O2.html"},
        {RESULT::C2H6O, "C2H6O.html"},
        {RESULT::CornOil, "Corn_oil.html"},
        {RESULT::EmptyBottle, "Bottle.html"},
        {RESULT::Empty, "Empty.html"},
        {RESULT::Starch, "Starch.html"},
        {RESULT::Sugar, "Sugar.html"},
        {RESULT::Salt, "Salt.html"},
        {RESULT::SodiumBicarbonate, "Sodium_bicarbonate.html"},
        {RESULT::TongSui, "Tong_sui.html"},
        {RESULT::OliveOil, "Olive_oil.html"},
        {RESULT::SesameOil, "Sesame_oil.html"},
        {RESULT::SunflowerOil, "Sunflower_oil.html"},
        {RESULT::WashingPowder, "Washing_powder.html"},
    };

    QString fileName = resultMap.value(result);
    if (fileName.isEmpty())
        return {};

    QString filePath = QDir(QCoreApplication::applicationDirPath()).filePath("page/" + fileName);

    return QUrl::fromLocalFile(filePath).toString();
}

void FormResult::showResult(RESULT result, const QString &name)
{
    ui->labelResult->setText(name);
    if (result == lastResult)
        return;
    lastResult = result;

    QString url = resultToLocalFileUrl(result);
    if (!url.isEmpty()) {
        ui->widget->load(QUrl(url));
    }
}

void FormResult::init()
{
    ui->widget->page()->runJavaScript(R"(
    document.querySelectorAll('a').forEach(el => {
        el.addEventListener('click', e => e.preventDefault());
    });
)");
    ui->widget->load(QUrl(resultToLocalFileUrl(RESULT::Empty)));
}
