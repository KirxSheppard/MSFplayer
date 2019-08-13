#include "brightnessdialog.hpp"
#include "ui_brightnessdialog.h"
#include <QBrush>

BrightnessDialog::BrightnessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BrightnessDialog)
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    ui->setupUi(this);
}

BrightnessDialog::~BrightnessDialog()
{
    delete ui;
}

void BrightnessDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    //background
    painter.fillRect(this->rect(),QColor::fromRgb(20, 20, 20, 255));

    //main background
    QRadialGradient gradient(70, 270,300, 700, 20);
        gradient.setColorAt(1, QColor::fromRgbF(0.15, 0.15, 0.15, 1));
        gradient.setColorAt(0, QColor::fromRgbF(0.3, 0.3, 0.3, 0.8));
        QBrush brush(gradient);
    painter.fillRect(5, 5, 290, 270, gradient);
}

void BrightnessDialog::on_horizontalSlider_valueChanged(int value)
{
    emit brightValue(value);
    QString temp = "Value: " + QString::number(value);
    ui->label_bright->setText(temp);

    sample_palette.setColor(QPalette::WindowText, Qt::white);
    ui->label_bright->setPalette(sample_palette);
}

void BrightnessDialog::on_horizontalSlider_R_valueChanged(int value)
{
    emit redValue(value);
    QString temp = "Value: " + QString::number(value);
    ui->label_2->setText(temp);
}

void BrightnessDialog::on_horizontalSlider_G_valueChanged(int value)
{
    emit greenValue(value);
    QString temp = "Value: " + QString::number(value);
    ui->label_3->setText(temp);
}

void BrightnessDialog::on_horizontalSlider_B_valueChanged(int value)
{
    emit blueValue(value);
    QString temp = "Value: " + QString::number(value);
    ui->label_4->setText(temp);
}

//Brightness check box
void BrightnessDialog::on_checkBox_clicked()
{
    mIfBright = !mIfBright;
    emit ifBrightBool(mIfBright);
}

//Red channel check box
void BrightnessDialog::on_checkBox_2_clicked()
{
    mIfRed = !mIfRed;
    emit ifRedChannel(mIfRed);
}

//Green channel check box
void BrightnessDialog::on_checkBox_3_clicked()
{
    mIfGreen = !mIfGreen;
    emit ifGreenChannel(mIfGreen);
}

//Blue channel check box
void BrightnessDialog::on_checkBox_4_clicked()
{
    mIfBlue = !mIfBlue;
    emit ifBlueChannel(mIfBlue);
}

void BrightnessDialog::on_horizontalSlider_sliderPressed()
{
    ui->horizontalSlider->setCursor(Qt::ClosedHandCursor);
}

void BrightnessDialog::on_horizontalSlider_sliderReleased()
{
    ui->horizontalSlider->setCursor(Qt::OpenHandCursor);
}

void BrightnessDialog::on_horizontalSlider_R_sliderPressed()
{
    ui->horizontalSlider_R->setCursor(Qt::ClosedHandCursor);
}

void BrightnessDialog::on_horizontalSlider_R_sliderReleased()
{
    ui->horizontalSlider_R->setCursor(Qt::OpenHandCursor);
}

void BrightnessDialog::on_horizontalSlider_G_sliderPressed()
{
    ui->horizontalSlider_G->setCursor(Qt::ClosedHandCursor);
}

void BrightnessDialog::on_horizontalSlider_G_sliderReleased()
{
    ui->horizontalSlider_G->setCursor(Qt::OpenHandCursor);
}

void BrightnessDialog::on_horizontalSlider_B_sliderPressed()
{
    ui->horizontalSlider_B->setCursor(Qt::ClosedHandCursor);
}

void BrightnessDialog::on_horizontalSlider_B_sliderReleased()
{
    ui->horizontalSlider_B->setCursor(Qt::OpenHandCursor);
}
