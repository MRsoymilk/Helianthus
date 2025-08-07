#include "mainwindow.h"
#include "../FormHistory/formhistory.h"
#include "../FormOTO/formoto.h"
#include "../FormPlot/formplot.h"
#include "../FormResult/formresult.h"
#include "../FormSerial/formserial.h"
#include "../FormSetting/formsetting.h"
#include "./ui_mainwindow.h"
#include "ThreadWorker/threadworker.h"
#include "g_define.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
    }
    delete m_panel;
    delete ui;
}

void MainWindow::init()
{
    m_panel = new FlowPanel(this);
    m_history = new FormHistory;
    m_plot = new FormPlot;
    m_result = new FormResult;
    m_serial = new FormSerial;
    m_oto = new FormOTO;
    m_setting = new FormSetting;

    ui->stackedWidget->addWidget(m_serial);
    ui->stackedWidget->addWidget(m_oto);
    ui->stackedWidget->addWidget(m_result);
    ui->stackedWidget->addWidget(m_plot);
    ui->stackedWidget->addWidget(m_history);
    ui->stackedWidget->addWidget(m_setting);

    ui->stackedWidget->setCurrentWidget(m_result);

    m_workerThread = new QThread(this);
    m_worker = new ThreadWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_serial, &FormSerial::recvSerialData, m_worker, &ThreadWorker::processData);
    connect(m_worker,
            &ThreadWorker::dataForPlotReady,
            m_plot,
            &FormPlot::updatePlot,
            Qt::QueuedConnection);
    connect(m_worker,
            &ThreadWorker::dataForTableReady,
            m_plot,
            &FormPlot::updateTable,
            Qt::QueuedConnection);
    connect(m_worker, &ThreadWorker::classificationForResult, m_result, &FormResult::showResult);
    connect(m_worker,
            &ThreadWorker::classificationForHistory,
            m_history,
            &FormHistory::recordHistory,
            Qt::QueuedConnection);
    connect(m_oto, &FormOTO::otoRequest, m_worker, &ThreadWorker::onOtoRequest);
    connect(m_worker, &ThreadWorker::otoRequestRaw, m_oto, &FormOTO::onOtoRequestRaw);

    m_workerThread->start();

    connect(m_panel, &FlowPanel::switchForm, this, &MainWindow::onSwitchForm);
    connect(m_panel, &FlowPanel::pinPanel, this, [&](bool isPin) {
        if (!isPin) {
            m_panel->updatePosition();
        }
    });

    QShortcut *shortcut_Serial = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_1), this);
    QShortcut *shortcut_OTO = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_2), this);
    QShortcut *shortcut_Result = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_3), this);
    QShortcut *shortcut_Plot = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_4), this);
    QShortcut *shortcut_History = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_5), this);
    connect(shortcut_Serial, &QShortcut::activated, this, [this]() { onSwitchForm(FORM::SERIAL); });
    connect(shortcut_OTO, &QShortcut::activated, this, [this]() { onSwitchForm(FORM::OTO); });
    connect(shortcut_Result, &QShortcut::activated, this, [this]() { onSwitchForm(FORM::RESULT); });
    connect(shortcut_Plot, &QShortcut::activated, this, [this]() { onSwitchForm(FORM::PLOT); });
    connect(shortcut_History, &QShortcut::activated, this, [this]() {
        onSwitchForm(FORM::HISTORY);
    });

    setTheme();
}

void MainWindow::setTheme()
{
    QString theme = SETTING_CONFIG_GET(CFG_GROUP_PROGRAM, CFG_PROGRAM_THEME, "OSX_Lite");

    QFile file(QString(":/res/themes/%1.qss").arg(theme));
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString style = file.readAll();
        qApp->setStyleSheet(style);
    }
    SETTING_CONFIG_SET(CFG_GROUP_PROGRAM, CFG_PROGRAM_THEME, theme);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    m_panel->updatePosition();
}

void MainWindow::onSwitchForm(FORM f)
{
    switch (f) {
    case FORM::HISTORY:
        ui->stackedWidget->setCurrentWidget(m_history);
        break;
    case FORM::PLOT:
        ui->stackedWidget->setCurrentWidget(m_plot);
        break;
    case FORM::RESULT:
        ui->stackedWidget->setCurrentWidget(m_result);
        break;
    case FORM::SERIAL:
        ui->stackedWidget->setCurrentWidget(m_serial);
        break;
    case FORM::SETTING:
        ui->stackedWidget->setCurrentWidget(m_setting);
        break;
    case FORM::OTO:
        ui->stackedWidget->setCurrentWidget(m_oto);
        break;
    }
}
