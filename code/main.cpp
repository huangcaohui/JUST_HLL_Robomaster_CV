#ifdef DEBUG
    /*Qt库*/
    #include "mainwindow.h"
#else
    /*主控制库*/
    #include "control.h"
#endif
    #include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

#ifdef DEBUG
    MainWindow instance;
    instance.show();
#else
    HCVC::Control instance;
#endif

    instance.run();

#ifdef DEBUG
    instance.close();


#else
    return 0;
#endif

    return a.exec();
}
