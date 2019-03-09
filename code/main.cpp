#ifdef DEBUG
    /*Qt库*/
    #include "mainwindow.h"
    #include <QApplication>
#else
    /*主控制库*/
    #include "control.h"
#endif

int main(int argc, char *argv[])
{

#ifdef DEBUG
    QApplication a(argc, argv);
    MainWindow instance;
    instance.show();
#else
    HCVC::Control instance;
#endif

    instance.run();

#ifdef DEBUG
    instance.close();

    return a.exec();
#else
    return 0;
#endif
}
