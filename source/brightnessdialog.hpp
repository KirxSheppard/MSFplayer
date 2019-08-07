#ifndef BRIGHTNESSDIALOG_HPP
#define BRIGHTNESSDIALOG_HPP

#include <QDialog>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

namespace Ui {
class BrightnessDialog;
}

class BrightnessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BrightnessDialog(QWidget *parent = nullptr);
    ~BrightnessDialog();

protected:
    void paintEvent(QPaintEvent *event);

signals:
    void ifBrightBool(bool mIfBright);
    void brightValue(int value);

    void ifRedChannel(bool mIfRed);
    void redValue(int value);

    void ifGreenChannel(bool mIfGreen);
    void greenValue(int value);

    void ifBlueChannel(bool mIrfBlue);
    void blueValue(int Value);

private slots:

    void on_horizontalSlider_sliderPressed();

    void on_horizontalSlider_sliderReleased();

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSlider_R_valueChanged(int value);

    void on_horizontalSlider_G_valueChanged(int value);

    void on_horizontalSlider_B_valueChanged(int value);

    void on_horizontalSlider_R_sliderPressed();

    void on_horizontalSlider_R_sliderReleased();

    void on_horizontalSlider_G_sliderPressed();

    void on_horizontalSlider_G_sliderReleased();

    void on_horizontalSlider_B_sliderPressed();

    void on_horizontalSlider_B_sliderReleased();

    void on_checkBox_clicked();

    void on_checkBox_2_clicked();

    void on_checkBox_3_clicked();

    void on_checkBox_4_clicked();

private:
    Ui::BrightnessDialog *ui;
    QString brightOptTitle;
    QPalette sample_palette;
    bool mIfBright = false;
    bool mIfRed = false;
    bool mIfGreen = false;
    bool mIfBlue = false;

};

#endif // BRIGHTNESSDIALOG_HPP
