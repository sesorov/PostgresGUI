#include "PostgresGUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PostgresGUI w;
    w.show();
    return a.exec();
}
