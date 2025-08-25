#ifndef FORMSELFTRAIN_H
#define FORMSELFTRAIN_H

#include <QWidget>
#include "mychartview.h"

namespace Ui {
class FormSelfTrain;
}

class FormSelfTrain : public QWidget
{
    Q_OBJECT

public:
    explicit FormSelfTrain(QWidget *parent = nullptr);
    ~FormSelfTrain();

signals:
    void startSelfTrainRecord(bool isDo);

public slots:
    void onDataRecv(const QList<QPointF> &v24,
                    const double &xMin,
                    const double &xMax,
                    const double &yMin,
                    const double &yMax);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void on_tBtnPrev_clicked();
    void on_tBtnNext_clicked();
    void on_tBtnRecord_clicked();
    void on_btnUpload_clicked();
    void onItemRenamed(QStandardItem *item);
    void on_tableViewFile_clicked(const QModelIndex &index);

private:
    void init();
    void addFile();
    void deleteFile();
    void clearFile();

    void clearAllData();
    void clearCurrentData();
    void writeAllData();

private:
    Ui::FormSelfTrain *ui;

private:
    QString m_data_dir;
    QList<QList<QPointF>> m_listV24;
    int m_current;
    int m_total;
    MyChartView *m_chartView = nullptr;
    QChart *m_chart = nullptr;
    QLineSeries *m_series;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    bool m_record;
    QStandardItemModel *m_model;
    QFile m_file;
    QList<QString> m_lastFile;
};

#endif // FORMSELFTRAIN_H
