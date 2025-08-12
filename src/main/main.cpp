#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include "g_define.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSharedMemory sharedMemory;
    sharedMemory.setKey("Helianthus_soymilk");

    if (!sharedMemory.create(1)) {
        QMessageBox::warning(nullptr, TITLE_WARNING, QObject::tr("Application is already running!"));

        return 0;
    }

    a.setWindowIcon(QIcon(":/res/icons/Helianthus.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
