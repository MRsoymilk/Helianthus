#ifndef FORMRESULT_H
#define FORMRESULT_H

#include <QWidget>
#include "g_define.h"

namespace Ui {
class FormResult;
}

class FormResult : public QWidget
{
    Q_OBJECT

public:
    explicit FormResult(QWidget *parent = nullptr);
    ~FormResult();

public slots:
    void showResult(RESULT result, const QString &name);

private:
    void init();

private:
    Ui::FormResult *ui;
    RESULT lastResult = RESULT::Empty;
};

#endif // FORMRESULT_H
