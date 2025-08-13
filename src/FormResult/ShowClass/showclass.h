#ifndef SHOWCLASS_H
#define SHOWCLASS_H

#include <QWidget>

namespace Ui {
class ShowClass;
}

class ShowClass : public QWidget
{
    Q_OBJECT

public:
    explicit ShowClass(QWidget *parent = nullptr);
    ~ShowClass();

private:
    Ui::ShowClass *ui;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
};

#endif // SHOWCLASS_H
