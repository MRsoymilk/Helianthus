#ifndef FORMHISTORY_H
#define FORMHISTORY_H

#include <QFile>
#include <QStandardItemModel>
#include <QWidget>
#include "g_define.h"

namespace Ui {
class FormHistory;
}

class FormHistory : public QWidget
{
    Q_OBJECT

public:
    explicit FormHistory(QWidget *parent = nullptr);
    ~FormHistory();

public slots:
    void recordHistory(const QJsonObject &obj);

private:
    void initModel();
    void initFile();
    void appendRecord(const QString &timestamp,
                      const QString &result,
                      const QVector<double> &voltages);
    QString resultToString(RESULT r);

private:
    Ui::FormHistory *ui;
    QStandardItemModel *m_model;
    QFile m_file;
    QTextStream m_stream;
    bool m_headerWritten = false;
};

#endif // FORMHISTORY_H
