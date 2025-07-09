#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "FlowPanel/flowpanel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class FormHistory;
class FormPlot;
class FormResult;
class FormSerial;
class FormSetting;
class ThreadWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onSwitchForm(FORM f);

private:
    void init();
    void setTheme();

private:
    Ui::MainWindow *ui;
    FlowPanel *m_panel;
    FormHistory *m_history;
    FormPlot *m_plot;
    FormResult *m_result;
    FormSerial *m_serial;
    FormSetting *m_setting;
    QThread *m_workerThread;
    ThreadWorker *m_worker;

protected:
    void resizeEvent(QResizeEvent *event);
};
#endif // MAINWINDOW_H
