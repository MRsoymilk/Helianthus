#ifndef FORMOTO_H
#define FORMOTO_H

#include <QWidget>

namespace Ui {
class FormOTO;
}

class FormOTO : public QWidget
{
    Q_OBJECT

public:
    explicit FormOTO(QWidget *parent = nullptr);
    ~FormOTO();

signals:
    void otoRequest(bool going, const QString &url);
    void otoCallParams();

public slots:
    void onOtoRequestRaw(const QJsonObject &obj);
private slots:

    void on_btnSendSpectrum_clicked();
    void on_btnSendInfo_clicked();

private:
    void init();

private:
    Ui::FormOTO *ui;
    bool m_switch;
};

#endif // FORMOTO_H
