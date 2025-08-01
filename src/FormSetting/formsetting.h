#ifndef FORMSETTING_H
#define FORMSETTING_H

#include <QWidget>

class AutoUpdate;

namespace Ui {
class FormSetting;
}

class FormSetting : public QWidget
{
    Q_OBJECT

public:
    explicit FormSetting(QWidget *parent = nullptr);
    ~FormSetting();

private:
    void init();

private:
    Ui::FormSetting *ui;
    AutoUpdate *m_update;
};

#endif // FORMSETTING_H
