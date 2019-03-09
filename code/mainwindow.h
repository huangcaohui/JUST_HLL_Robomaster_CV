#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*Qt库*/
#include <QMainWindow>

/*自定义库*/
#include "control.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public HCVC::Control
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_horizontalScrollBar_redMin_valueChanged(int value);

    void on_horizontalScrollBar_greenMin_valueChanged(int value);

    void on_horizontalScrollBar_blueMin_valueChanged(int value);

    void on_horizontalScrollBar_redMax_valueChanged(int value);

    void on_horizontalScrollBar_greenMax_valueChanged(int value);

    void on_horizontalScrollBar_blueMax_valueChanged(int value);

    void on_angleRange_valueChanged(double arg1);

    void on_minArea_valueChanged(double arg1);

    void on_maxHeightWidthRat_valueChanged(double arg1);

    void on_minHeightWidthRat_valueChanged(double arg1);

    void on_tanAngle_valueChanged(double arg1);

    void on_deviationAngle_valueChanged(double arg1);

    void on_armourPixelAvg_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
