#include "formselftrain.h"
#include "ui_formselftrain.h"

#include <QFileDialog>
#include <QMenu>
#include <QStandardItemModel>
#include "g_define.h"
#include "myhttp.h"

FormSelfTrain::FormSelfTrain(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormSelfTrain)
{
    ui->setupUi(this);
    init();
}

FormSelfTrain::~FormSelfTrain()
{
    delete ui;
}

void FormSelfTrain::onDataRecv(const QList<QPointF> &v24,
                               const double &xMin,
                               const double &xMax,
                               const double &yMin,
                               const double &yMax)
{
    m_listV24.append(v24);
    m_series->replace(v24);
    m_axisX->setRange(xMin, xMax);
    m_axisY->setRange(yMin, yMax);
    m_current = m_total = m_listV24.size();
    ui->labelIndex->setText(QString("%1/%2").arg(m_current).arg(m_total));
}

void FormSelfTrain::init()
{
    m_current = 0;
    m_record = false;

    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({"File"});
    ui->tableViewFile->setEditTriggers(QAbstractItemView::DoubleClicked
                                       | QAbstractItemView::SelectedClicked);
    ui->tableViewFile->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewFile->setModel(m_model);
    connect(m_model, &QStandardItemModel::itemChanged, this, &FormSelfTrain::onItemRenamed);

    ui->tableViewFile->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableViewFile, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos) {
        QMenu menu;
        QAction *actionAdd = menu.addAction("Add");
        QAction *actionDelete = menu.addAction("Delete");
        QAction *actionClearAll = menu.addAction("Clear All");
        connect(actionAdd, &QAction::triggered, this, &FormSelfTrain::addFile);
        connect(actionDelete, &QAction::triggered, this, &FormSelfTrain::deleteFile);
        connect(actionClearAll, &QAction::triggered, this, &FormSelfTrain::clearFile);
        menu.exec(ui->tableViewFile->viewport()->mapToGlobal(pos));
    });
    ui->tBtnRecord->setCheckable(true);

    m_series = new QLineSeries();
    m_series->setColor(Qt::blue);
    m_chart = new QChart();
    m_chart->addSeries(m_series);
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_axisX->setTitleText("Index");
    m_axisX->setRange(0, 0.2);
    m_axisY->setTitleText("Voltage (V)");
    m_axisY->setRange(0, 0.2);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setTitle("History Plot");

    m_chartView = new MyChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    ui->gLayPlot->addWidget(m_chartView);

    m_chartView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_chartView, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos) {
        QMenu menu(this);
        QAction *clearCurrentAction = menu.addAction(tr("Clear Current"));
        QAction *clearAllAction = menu.addAction(tr("Clear All"));
        QAction *writeAllAction = menu.addAction(tr("Write All"));
        connect(clearCurrentAction, &QAction::triggered, this, &FormSelfTrain::clearCurrentData);
        connect(clearAllAction, &QAction::triggered, this, &FormSelfTrain::clearAllData);
        connect(writeAllAction, &QAction::triggered, this, &FormSelfTrain::writeAllData);
        menu.exec(m_chartView->mapToGlobal(pos));
    });
    ui->textBrowserTrainInfo->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->textBrowserTrainInfo,
            &QWidget::customContextMenuRequested,
            this,
            [this](const QPoint &pos) {
                QMenu menu;
                QAction *clearAction = menu.addAction(tr("Clear"));
                connect(clearAction, &QAction::triggered, this, [this]() {
                    ui->textBrowserTrainInfo->clear();
                });
                menu.exec(ui->textBrowserTrainInfo->mapToGlobal(pos));
            });

    QStringList methods;
    methods << "knn_reflection" << "knn_transmission" << "regression";
    ui->comboBoxMethod->addItems(methods);
}

void FormSelfTrain::onItemRenamed(QStandardItem *item)
{
    QSignalBlocker blocker(m_model);
    if (!item) {
        return;
    }

    QString oldFilePath = m_lastFile.at(item->index().row());
    QString newFilePath = item->text();

    QFileInfo fi(oldFilePath);

    if (QFile::exists(newFilePath)) {
        QMessageBox::warning(this, TITLE_WARNING, "target file already exist!");
        item->setText(fi.fileName());
        return;
    }

    if (QFile::rename(oldFilePath, newFilePath)) {
        item->setData(newFilePath, Qt::UserRole + 1);
        qDebug() << "file rename: " << oldFilePath << "->" << newFilePath;
        m_lastFile[item->index().row()] = newFilePath;
    } else {
        QMessageBox::warning(this, TITLE_WARNING, "unable to rename file!");
        item->setText(fi.fileName());
    }
}

void FormSelfTrain::addFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Create file");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            QList<QStandardItem *> row;
            row << new QStandardItem(fileName);
            m_model->appendRow(row);
            m_lastFile.append(fileName);
        } else {
            QMessageBox::warning(this, TITLE_WARNING, "Failed to create file: " + fileName);
        }
    }
}

void FormSelfTrain::deleteFile()
{
    QModelIndex currentIndex = ui->tableViewFile->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, TITLE_WARNING, "Please choose a file to delete");
        return;
    }

    int row = currentIndex.row();
    QString filePath = m_lastFile.at(row);

    QFile file(filePath);
    if (!file.remove()) {
        QMessageBox::warning(this, TITLE_WARNING, "Failed to delete file: " + filePath);
        return;
    }

    m_model->removeRow(row);

    m_lastFile.removeAt(row);
}

void FormSelfTrain::clearFile()
{
    if (m_model->rowCount() == 0)
        return;

    for (const QString &filePath : m_lastFile) {
        QFile file(filePath);
        if (!file.remove()) {
            qDebug() << "delete failed:" << filePath;
        }
    }

    m_model->removeRows(0, m_model->rowCount());

    m_lastFile.clear();
}

void FormSelfTrain::clearAllData()
{
    m_current = m_total = 0;
    m_series->clear();
    m_listV24.clear();
    ui->labelIndex->setText(QString("%1/%2").arg(m_current).arg(m_total));
}

void FormSelfTrain::clearCurrentData()
{
    if (m_current < 0 || m_current > m_listV24.size()) {
        return;
    }
    m_total--;
    m_series->replace(m_listV24[m_current - 1]);
    m_listV24.remove(m_current - 1);
    if (m_current > m_total) {
        m_current--;
    }
    ui->labelIndex->setText(QString("%1/%2").arg(m_current).arg(m_total));
}

void FormSelfTrain::on_tBtnPrev_clicked()
{
    if (m_current - 1 > 0) {
        m_current--;
        ui->labelIndex->setText(QString("%1/%2").arg(m_current).arg(m_total));
        m_series->replace(m_listV24[m_current - 1]);
    }
}

void FormSelfTrain::on_tBtnNext_clicked()
{
    if (m_current + 1 <= m_total) {
        m_current++;
        ui->labelIndex->setText(QString("%1/%2").arg(m_current).arg(m_total));
        m_series->replace(m_listV24[m_current - 1]);
    }
}

void FormSelfTrain::on_tBtnRecord_clicked()
{
    m_record = !m_record;
    emit startSelfTrainRecord(m_record);
}

void FormSelfTrain::on_btnUpload_clicked()
{
    if (m_model->rowCount() == 0) {
        QMessageBox::warning(this, TITLE_WARNING, "No File!");
        return;
    }

    QStringList filePaths;
    for (const QString &filePath : m_lastFile) {
        filePaths.append(filePath);
    }

    QString field = "file";
    QMap<QString, QString> otherFields;
    QString method = ui->comboBoxMethod->currentText();
    otherFields.insert("method", method);
    otherFields.insert("folder", TIMESTAMP_1("MMdd"));

    MyHttp *http = new MyHttp(this);
    http->uploadFiles(
        QUrl("http://192.168.123.233:5050/upload"),
        filePaths,
        field,
        otherFields,
        [this](const QJsonObject &obj) {
            ui->textBrowserTrainInfo->append(QString("[%1] start train").arg(TIMESTAMP_0()));
            ui->textBrowserTrainInfo->append("upload success");
            QObject *senderObj = sender();
            if (senderObj) {
                senderObj->deleteLater();
            }
            QString folderName = QFileInfo(obj["saved_folder"].toString()).fileName();

            QTimer *timer = new QTimer(this);
            MyHttp *http = new MyHttp(this);

            connect(timer, &QTimer::timeout, this, [this, timer, http, folderName]() {
                QJsonObject status = http->get_sync(
                    QString("http://192.168.123.233:5050/train_status/%1").arg(folderName));
                QString err = status["stderr"].toString();
                QString out = status["stdout"].toString();
                if (!err.isEmpty()) {
                    ui->textBrowserTrainInfo->append(QString("[Error]: %1").arg(err));
                }
                if (!out.isEmpty()) {
                    ui->textBrowserTrainInfo->append(QString("[Info ]: %2").arg(out));
                }

                if (status["status"] != "running") {
                    timer->stop();
                }
            });

            connect(http,
                    &MyHttp::jsonResponse,
                    this,
                    [this, timer, http](const QJsonObject &statusObj) {
                        QString status = statusObj["status"].toString();
                        ui->textBrowserTrainInfo->append("Training status: " + status);

                        if (status == "finished" || status == "failed") {
                            timer->stop();
                            timer->deleteLater();
                            http->deleteLater();
                            ui->textBrowserTrainInfo->append("Training completed.");
                        }
                    });

            connect(http, &MyHttp::httpError, this, [this, timer, http](const QString &error) {
                ui->textBrowserTrainInfo->append("Error checking status: " + error);
                timer->stop();
                timer->deleteLater();
                http->deleteLater();
            });
            timer->start(1000);
        },
        [this](const QString &error) {
            ui->textBrowserTrainInfo->append("upload failed: " + error);
            QObject *senderObj = sender();
            if (senderObj) {
                senderObj->deleteLater();
            }
        },
        [this](qint64 sent, qint64 total) {
            if (total != 0) {
                ui->textBrowserTrainInfo->append(
                    QString("upload progress: %1%").arg(1.0 * sent / total * 100));
                ui->tabWidget->setCurrentWidget(ui->tabTrain);
            }
        });
}

void FormSelfTrain::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("choose data folder"),
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        qDebug() << "folder: " << dir;
        ui->labelFolder->setText(dir);
        m_data_dir = dir;
        QDir directory(dir);
        QStringList filters;
        filters << "*.csv";
        QFileInfoList fileList = directory.entryInfoList(filters, QDir::Files);

        for (const QFileInfo &fileInfo : fileList) {
            QString fileName = fileInfo.absoluteFilePath();
            QList<QStandardItem *> row;
            row << new QStandardItem(fileName);
            m_model->appendRow(row);
            m_lastFile.append(fileName);
        }
    } else {
        qDebug() << "cancel!";
    }
}

void FormSelfTrain::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_model->removeRows(0, m_model->rowCount());
    m_lastFile.clear();
}

void FormSelfTrain::on_tableViewFile_clicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QString path = index.data(Qt::DisplayRole).toString();
    QFileInfo info(path);
    ui->labelFile->setText(info.fileName());
    ui->tabWidget->setCurrentWidget(ui->tabDataGet);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, TITLE_WARNING, "cannot open file: " + path);
        return;
    }

    QTextStream in(&file);

    m_listV24.clear();

    if (in.atEnd()) {
        file.close();
        m_current = m_total = 0;
        ui->labelIndex->setText(QString("%1/%2").arg(m_current).arg(m_total));
        return;
    }

    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(',', Qt::SkipEmptyParts);

    if (headers.size() < 2) {
        QMessageBox::warning(this, TITLE_WARNING, "invalid file format");
        file.close();
        return;
    }

    int nCurves = headers.size() - 1;
    m_listV24.resize(nCurves);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() < 2)
            continue;

        bool okX;
        double x = parts[0].toDouble(&okX);
        if (!okX)
            continue;

        for (int i = 0; i < nCurves; ++i) {
            if (i + 1 >= parts.size())
                continue;

            bool okY;
            double y = parts[i + 1].toDouble(&okY);
            if (okY)
                m_listV24[i].append(QPointF(x, y));
        }
    }

    file.close();

    if (!m_listV24.isEmpty()) {
        auto curve = m_listV24.last();
        m_series->replace(curve);
        m_current = m_listV24.size() - 1;
        m_total = m_listV24.size();
        double yMin = std::numeric_limits<double>::max();
        double yMax = std::numeric_limits<double>::lowest();
        m_axisX->setRange(curve.first().x(), curve.last().x());
        for (int i = 0; i < curve.size(); ++i) {
            yMin = qMin(curve.at(i).y(), yMin);
            yMax = qMax(curve.at(i).y(), yMax);
        }
        m_axisY->setRange(yMin, yMax);
        ui->labelIndex->setText(QString("%1/%2").arg(m_current + 1).arg(m_total));
    }
}

void FormSelfTrain::writeAllData()
{
    if (m_listV24.isEmpty() || m_lastFile.isEmpty()) {
        QMessageBox::warning(this, TITLE_WARNING, "no data to save");
        return;
    }

    QString filePath = QString("%1/%2").arg(ui->labelFolder->text(), ui->labelFile->text());

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "can not write file:" << filePath;
        return;
    }

    QTextStream out(&file);
    out << "x";
    for (int i = 0; i < m_listV24.size(); ++i) {
        out << ",curve_" << i + 1;
    }
    out << "\n";

    int rows = m_listV24.at(0).length();

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < m_listV24.size(); ++col) {
            const QList<QPointF> &curve = m_listV24[col];
            if (col == 0) {
                double x = (row < curve.size()) ? curve[row].x() : row;
                out << x;
            }
            double y = (row < curve.size()) ? curve[row].y() : 0.0;
            out << "," << y;
        }
        out << "\n";
    }

    file.close();

    qDebug() << "save:" << filePath;

    QMessageBox::information(this, TITLE_INFO, "all data saved!");
}
