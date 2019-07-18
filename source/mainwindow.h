#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setImage(const QImage &img);

private:
    int height, width;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QImage m_img;
};

#endif // MAINWINDOW_H
