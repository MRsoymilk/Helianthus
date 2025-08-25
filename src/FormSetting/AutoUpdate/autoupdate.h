#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QJsonObject>
#include <QWidget>

class MyHttp;

namespace Ui {
class AutoUpdate;
}

class AutoUpdate : public QWidget
{
    Q_OBJECT

public:
    explicit AutoUpdate(QWidget *parent = nullptr);
    ~AutoUpdate();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_btnUpdate_clicked();

private:
    QJsonObject checkUpdate();
    bool isNewVersion();

private:
    Ui::AutoUpdate *ui;
    QJsonObject m_objUpdate;
    QString m_url;
    MyHttp *m_http;
};

#endif // AUTOUPDATE_H
