#include <QtGui/QApplication>
#include "mainwindow.h"
#include "canvas.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    Canvas canvas(NULL);
    w.setCentralWidget(&canvas);

    w.show();

    return a.exec();
}
