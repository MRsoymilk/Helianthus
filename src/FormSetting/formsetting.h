#ifndef FORMSETTING_H
#define FORMSETTING_H

#include <QWidget>

class AutoUpdate;
class SerialFrame;
class TrainSetting;
class ClassifySetting;
class SeparationSetting;

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
    SerialFrame *m_frame;
    TrainSetting *m_train;
    ClassifySetting *m_classify;
    SeparationSetting *m_separation;
};

#endif // FORMSETTING_H
