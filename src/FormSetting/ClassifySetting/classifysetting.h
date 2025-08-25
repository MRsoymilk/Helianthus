#ifndef CLASSIFYSETTING_H
#define CLASSIFYSETTING_H

#include <QWidget>

namespace Ui {
class ClassifySetting;
}

class ClassifySetting : public QWidget
{
    Q_OBJECT

public:
    explicit ClassifySetting(QWidget *parent = nullptr);
    ~ClassifySetting();

private slots:
    void on_btnModify_clicked();

private:
    void init();

private:
    Ui::ClassifySetting *ui;
};

#endif // CLASSIFYSETTING_H
