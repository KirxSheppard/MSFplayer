#ifndef INSPECTOR_HPP
#define INSPECTOR_HPP

#include <QScrollArea>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QDragEnterEvent>
#include <QMimeData>

namespace Ui {
class Inspector;
}

class Inspector : public QWidget
{
    Q_OBJECT

public:
    explicit Inspector(QWidget *parent = nullptr);
    ~Inspector();

    void resetWm2BigPic();

protected:
    void paintEvent(QPaintEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void ifBrightBool(bool mIfBright);
    void brightValue(int value);

    void ifRedChannel(bool mIfRed);
    void redValue(int value);

    void ifGreenChannel(bool mIfGreen);
    void greenValue(int value);

    void ifBlueChannel(bool mIrfBlue);
    void blueValue(int Value);

    void wmOpacityValue(double value);
    void wmPosX(int value);
    void wmPosY(int value);
    void wmResetPath(bool value);
    void wmSetPressed(QString path);
    void wmScaleValue(int value);

    void osOpacityValue(double value);

private slots:

    void on_horizontalSlider_Br_sliderPressed();

    void on_horizontalSlider_Br_sliderReleased();

    void on_horizontalSlider_Br_valueChanged(int value);

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

    void on_horizontalSlider_wm_opacity_sliderPressed();

    void on_horizontalSlider_wm_opacity_sliderReleased();

    void on_horizontalSlider_wm_opacity_valueChanged(int value);

    void on_pushButtonWmReset_clicked();

    void on_pushButtonWmSet_clicked();

    void on_horizontalSlider_wmSize_valueChanged(int value);

    void on_horizontalSlider_wmSize_sliderPressed();

    void on_horizontalSlider_wmSize_sliderReleased();

    void on_horizontalSliderOsOpacity_valueChanged(int value);

    void on_horizontalSliderOsOpacity_sliderPressed();

    void on_horizontalSliderOsOpacity_sliderReleased();

    void on_spinBox_X_editingFinished();
    void on_spinBox_Y_editingFinished();

private:
    Ui::Inspector *ui;
    QString brightOptTitle;
    QPalette sample_palette;
    bool mIfBright = false;
    bool mIfRed = false;
    bool mIfGreen = false;
    bool mIfBlue = false;
    QString mFileWithFormat;
    QString mFileName;
};

#endif // INSPECTOR_HPP
