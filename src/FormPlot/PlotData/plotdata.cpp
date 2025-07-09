#include "plotdata.h"
#include <QFileDialog>
#include <QMenu>
#include "ui_plotdata.h"

PlotData::PlotData(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlotData)
{
    ui->setupUi(this);
    init();
}

PlotData::~PlotData()
{
    delete ui;
}

void PlotData::init()
{
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({"vol24", "raw24"});
    ui->tableView->setModel(m_model);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos) {
        QMenu menu;
        QAction *exportAction = menu.addAction("Export to CSV");
        connect(exportAction, &QAction::triggered, this, &PlotData::exportToCSV);
        QAction *exportAllAction = menu.addAction("Export All to CSV");
        connect(exportAllAction, &QAction::triggered, this, &PlotData::exportAllToCSV);
        menu.exec(ui->tableView->viewport()->mapToGlobal(pos));
    });
}

void PlotData::updateTable(const QVector<double> &v24, const QVector<qint32> &raw24)
{
    if (this->isVisible()) {
        m_listV24.push_back(v24);
        m_listRaw24.push_back(raw24);
        m_current = m_listV24.size();
        m_total = m_listV24.size();
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));

        // m_model->removeRows(0, m_model->rowCount());

        // int count = std::min(v24.size(), raw24.size());
        // for (int i = 0; i < count; ++i) {
        //     QList<QStandardItem *> row;
        //     row << new QStandardItem(QString::number(v24[i], 'f', 6));
        //     row << new QStandardItem(QString::number(raw24[i]));
        //     m_model->appendRow(row);
        // }
        fillData(v24, raw24);
    }
}

void PlotData::fillData(const QVector<double> &v24, const QVector<qint32> &raw24)
{
    m_model->removeRows(0, m_model->rowCount());

    int count = std::min(v24.size(), raw24.size());
    for (int i = 0; i < count; ++i) {
        QList<QStandardItem *> row;
        row << new QStandardItem(QString::number(v24[i], 'f', 6));
        row << new QStandardItem(QString::number(raw24[i]));
        m_model->appendRow(row);
    }
}

void PlotData::exportToCSV()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("save to CSV file"),
                                                "data.csv",
                                                tr("CSV file (*.csv)"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "vol24,raw24\n";

    for (int row = 0; row < m_model->rowCount(); ++row) {
        QString v = m_model->item(row, 0)->text();
        QString r = m_model->item(row, 1)->text();
        out << v << "," << r << "\n";
    }

    file.close();
}

void PlotData::exportAllToCSV()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save All to CSV"),
                                                "data_all.csv",
                                                tr("CSV file (*.csv)"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("Failed to open file: %s", qPrintable(path));
        return;
    }

    QTextStream out(&file);

    int batchCount = std::min(m_listV24.size(), m_listRaw24.size());

    for (int i = 0; i < batchCount; ++i) {
        out << QString("vol24_%1,raw24_%1").arg(i + 1);
        if (i < batchCount - 1)
            out << ",";
    }
    out << "\n";

    int maxRow = 0;
    for (int i = 0; i < batchCount; ++i)
        maxRow = qMax(maxRow, qMin(m_listV24[i].size(), m_listRaw24[i].size()));

    for (int row = 0; row < maxRow; ++row) {
        for (int batch = 0; batch < batchCount; ++batch) {
            const QVector<double> &v24 = m_listV24[batch];
            const QVector<qint32> &raw24 = m_listRaw24[batch];

            if (row < v24.size() && row < raw24.size()) {
                out << QString::number(v24[row], 'f', 6) << "," << QString::number(raw24[row]);
            } else {
                out << ",";
            }

            if (batch < batchCount - 1)
                out << ",";
        }
        out << "\n";
    }

    file.close();
}

void PlotData::closeEvent(QCloseEvent *event)
{
    m_listV24.clear();
    m_listRaw24.clear();
    m_current = m_total = 0;
    ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));
    emit plotDataClose();
    QWidget::closeEvent(event);
}

void PlotData::on_tBtnPrev_clicked()
{
    if (m_current - 1 > 0) {
        m_current--;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));
        fillData(m_listV24[m_current - 1], m_listRaw24[m_current - 1]);
    }
}

void PlotData::on_tBtnNext_clicked()
{
    if (m_current + 1 <= m_total) {
        m_current++;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));
        fillData(m_listV24[m_current - 1], m_listRaw24[m_current - 1]);
    }
}

void PlotData::on_lineEditGo_editingFinished()
{
    int val = ui->lineEditGo->text().toInt();
    if (val > 0 && val <= m_total) {
        m_current = val;
        ui->labelStatus->setText(QString("%1/%2").arg(m_current).arg(m_total));
        fillData(m_listV24[m_current - 1], m_listRaw24[m_current - 1]);
    }
}
