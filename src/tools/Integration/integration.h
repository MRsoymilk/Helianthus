#ifndef INTEGRATION_H
#define INTEGRATION_H

#include <QWidget>

namespace Ui {
class Integration;
}

class Integration : public QWidget
{
    Q_OBJECT

public:
    explicit Integration(QWidget *parent = nullptr);
    ~Integration();

private slots:
    void on_btnConversion_clicked();

private:
    Ui::Integration *ui;
    QString generateHex(int times);
    int getTimes(const QString &hexString);
};

#endif // INTEGRATION_H
