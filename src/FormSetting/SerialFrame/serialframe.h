#ifndef SERIALFRAME_H
#define SERIALFRAME_H

#include <QWidget>

namespace Ui {
class SerialFrame;
}

class SerialFrame : public QWidget
{
    Q_OBJECT

public:
    explicit SerialFrame(QWidget *parent = nullptr);
    ~SerialFrame();

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_btnModify_clicked();

private:
    Ui::SerialFrame *ui;
};

#endif // SERIALFRAME_H
