#ifndef PLOTDATA_H
#define PLOTDATA_H

#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class PlotData;
}

class PlotData : public QWidget
{
    Q_OBJECT
public:
    explicit PlotData(QWidget *parent = nullptr);
    ~PlotData();

signals:
    void plotDataClose();

public slots:
    void updateTable(const QVector<double> &v24, const QVector<qint32> &raw24);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_tBtnPrev_clicked();
    void on_tBtnNext_clicked();
    void on_lineEditGo_editingFinished();

private:
    void init();
    void exportToCSV();
    void exportAllToCSV();
    void fillData(const QVector<double> &v24, const QVector<qint32> &raw24);

private:
    int m_current = 0;
    int m_total = 0;
    QList<QVector<double>> m_listV24;
    QList<QVector<qint32>> m_listRaw24;
    Ui::PlotData *ui;
    QStandardItemModel *m_model;
};

#endif // PLOTDATA_H
