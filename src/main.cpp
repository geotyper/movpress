#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("MovPress");
    QApplication::setOrganizationName("geotyper");

    MainWindow window;
    window.show();
    return app.exec();
}
