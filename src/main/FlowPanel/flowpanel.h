#ifndef FLOWPANEL_H
#define FLOWPANEL_H

#include <QPropertyAnimation>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include "g_define.h"

namespace Ui {
class FlowPanel;
}

class FlowPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FlowPanel(QWidget *parent = nullptr);
    ~FlowPanel();

signals:
    void switchForm(FORM f);
    void pinPanel(bool isPin);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

public slots:
    void updatePosition();

private slots:
    void on_btnSerial_clicked();
    void on_btnOTO_clicked();
    void on_btnResult_clicked();
    void on_btnPlot_clicked();
    void on_btnHistory_clicked();
    void on_tBtnPin_clicked();
    void on_btnSetting_clicked();
    void on_btnSelfTrain_clicked();

private:
    void init();

private:
    Ui::FlowPanel *ui;
    QPropertyAnimation *animation;
    int m_visual_size = 20;
    bool m_pinned = false;
};
#endif // FLOWPANEL_H
