#include "formhistory.h"
#include "ui_formhistory.h"

#include <QDir>
#include <numeric>

FormHistory::FormHistory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormHistory)
    , m_model(new QStandardItemModel(this))
    , m_file("data/history.csv")
    , m_stream(&m_file)
{
    ui->setupUi(this);
    initModel();
    initFile();
}

FormHistory::~FormHistory()
{
    if (m_file.isOpen()) {
        m_file.close();
    }
    delete ui;
}

void FormHistory::initModel()
{
    m_model->setHorizontalHeaderLabels({"timestamp", "result", "elapsed(ms)"});
    ui->tableView->setModel(m_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void FormHistory::initFile()
{
    QDir dir("data");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    if (m_file.open(QIODevice::Append | QIODevice::Text)) {
        if (m_file.size() == 0) {
            m_headerWritten = false;
        } else {
            m_headerWritten = true;
        }
    } else {
        qWarning() << "Cannot open history file: " << m_file.errorString();
    }
}

void FormHistory::recordHistory(const QJsonObject &obj)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString result = obj.value("result").toString();
    double elapsed = obj.value("elapsed").toDouble();

    // 显示在表格中
    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(timestamp) << new QStandardItem(result)
             << new QStandardItem(QString::number(elapsed, 'f', 3));
    m_model->appendRow(rowItems);

    if (m_file.isOpen()) {
        // 写 header（仅第一次写）
        if (!m_headerWritten) {
            m_stream << "timestamp,result,elapsed(ms)\n";
            m_stream.flush();
            m_headerWritten = true;
        }

        // 写数据行
        m_stream << timestamp << "," << result << "," << QString::number(elapsed, 'f', 6) << "\n";
        m_stream.flush();
    }
}
