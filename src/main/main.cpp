#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/res/icons/Helianthus.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
