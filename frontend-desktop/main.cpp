#include "mainwindow.h"
#include <QApplication>
// #include <QDirIterator>

int main(int argc, char *argv[])
{
    //Create an iterator for the :/resources directory and its subdirectories.
    // QDirIterator it(":", QDirIterator::Subdirectories);
    // while (it.hasNext()) {
    //     QString resourcePath = it.next();
    //     qDebug() << resourcePath;
    // }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
