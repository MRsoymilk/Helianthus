#ifndef SEPARATIONSETTING_H
#define SEPARATIONSETTING_H

#include <QWidget>

namespace Ui {
class SeparationSetting;
}

class SeparationSetting : public QWidget
{
    Q_OBJECT

public:
    explicit SeparationSetting(QWidget *parent = nullptr);
    ~SeparationSetting();

private slots:
    void on_btnModify_clicked();

private:
    void init();

private:
    Ui::SeparationSetting *ui;
};

#endif // SEPARATIONSETTING_H
