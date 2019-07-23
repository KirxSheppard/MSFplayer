#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString fileName, int numofFrames, QWidget *parent = nullptr);
    ~MainWindow();

    void setImage(const QImage &img);
    void setSliderValue(int numofFrames,int valueS);
//    int changeFrame();
    int setFrame();
    bool checkifExit();
    bool checkifSave();



protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void on_horizontalSlider_valueChanged(int value);
    void on_actionexit_triggered();

private slots:
    void on_actionSave_triggered();

private:
    Ui::MainWindow *ui;
    QImage m_img;
    bool ifExit, ifSave;
    int height, width;
    int numOfFrames;
};

#endif // MAINWINDOW_H
