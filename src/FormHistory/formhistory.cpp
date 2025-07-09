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
    m_model->setHorizontalHeaderLabels({"timestamp", "result", "average vol"});
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
            m_headerWritten = false; // 延迟写 header（知道 voltages 数量后再写）
        } else {
            m_headerWritten = true;
        }
    } else {
        qWarning() << "Cannot open history file: " << m_file.errorString();
    }
}

void FormHistory::appendRecord(const QString &timestamp,
                               const QString &result,
                               const QVector<double> &voltages)
{
    // 表格中只显示平均值
    double average = std::accumulate(voltages.begin(), voltages.end(), 0.0) / voltages.size();
    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(timestamp) << new QStandardItem(result)
             << new QStandardItem(QString::number(average, 'f', 3));
    m_model->appendRow(rowItems);

    if (m_file.isOpen()) {
        // 写 header（仅第一次写）
        if (!m_headerWritten) {
            m_stream << "timestamp,result";
            for (int i = 0; i < voltages.size(); ++i) {
                m_stream << ",vol_" << (i + 1);
            }
            m_stream << "\n";
            m_stream.flush();
            m_headerWritten = true;
        }

        // 写数据行
        m_stream << timestamp << "," << result;
        for (const double &v : voltages) {
            m_stream << "," << QString::number(v, 'f', 6);
        }
        m_stream << "\n";
        m_stream.flush();
    }
}

QString FormHistory::resultToString(RESULT r)
{
    switch (r) {
    case RESULT::Starch:
        return "STARCH";
    case RESULT::Sugar:
        return "SUGAR";
    default:
        return "EMPTY";
    }
}

void FormHistory::recordHistory(const QJsonObject &obj)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    // QString resultStr = resultToString(result);
    qDebug() << obj;
    // appendRecord(timestamp, resultStr, v24);
}
