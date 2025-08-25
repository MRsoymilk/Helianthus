#ifndef TRAINSETTING_H
#define TRAINSETTING_H

#include <QWidget>

namespace Ui {
class TrainSetting;
}

class TrainSetting : public QWidget
{
    Q_OBJECT

public:
    explicit TrainSetting(QWidget *parent = nullptr);
    ~TrainSetting();

private slots:
    void on_btnModify_clicked();

private:
    void init();

private:
    Ui::TrainSetting *ui;
};

#endif // TRAINSETTING_H
