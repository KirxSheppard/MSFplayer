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
    bool checkifPaused();
    bool checkWaterMark();



protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void on_horizontalSlider_valueChanged(int value);
    void on_actionexit_triggered();

private slots:
    void on_actionSave_triggered();
    void on_playPauseButton_clicked();

    void on_actionAbout_this_app_triggered();

    void on_actionwater_mark_triggered();

private:
    Ui::MainWindow *ui;
    QImage m_img;
    bool ifExit, ifSave, ifPaused, ifWaterMark;
    int height, width;
    int numOfFrames, frameSaveCounter;
};

#endif // MAINWINDOW_H
