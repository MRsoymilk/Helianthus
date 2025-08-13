#include "showclass.h"
#include "myhttp.h"
#include "ui_showclass.h"

#include <QJsonDocument>
#include <QShowEvent>

ShowClass::ShowClass(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowClass)
{
    ui->setupUi(this);
}

ShowClass::~ShowClass()
{
    delete ui;
}

void ShowClass::showEvent(QShowEvent *event)
{
    MyHttp http;
    QJsonObject obj = http.get_sync("http://192.168.123.233:5010/classes");
    QJsonDocument doc(obj);
    QString jsonStr = doc.toJson(QJsonDocument::Indented);

    ui->textBrowserClass->setText(jsonStr);
}
