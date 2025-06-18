#include "securex.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    secureX w;
    w.show();
    return a.exec();
}
