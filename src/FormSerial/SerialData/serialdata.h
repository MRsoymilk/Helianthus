#ifndef SERIALDATA_H
#define SERIALDATA_H

#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class SerialData;
}

class SerialData : public QWidget
{
    Q_OBJECT

public:
    explicit SerialData(QWidget *parent = nullptr);
    ~SerialData();
signals:
    void serialDataClose();
public slots:
    void onSerialDataReceive(const QByteArray &data);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::SerialData *ui;
    QStandardItemModel *m_model;
};

#endif // SERIALDATA_H
