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
class FormOTO;
class FormSetting;
class FormSelfTrain;
class ThreadWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onSwitchForm(FORM f);

protected:
    void resizeEvent(QResizeEvent *event);

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
    FormOTO *m_oto;
    FormSetting *m_setting;
    FormSelfTrain *m_selfTrain;
    QThread *m_workerThread;
    ThreadWorker *m_worker;
};
#endif // MAINWINDOW_H
