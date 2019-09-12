#include "inspector.hpp"
#include "ui_inspector.h"
#include <QBrush>

Inspector::Inspector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inspector)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->groupBoxColor->setStyleSheet("color: white");
    ui->groupBoxWaterMark->setStyleSheet("color: white");
    ui->spinBox_X->setStyleSheet("background: gray");
    ui->spinBox_X->setRange(-10000, 10000);
    ui->spinBox_Y->setStyleSheet("background: gray");
    ui->spinBox_Y->setRange(-10000, 10000);
    ui->checkBox->setStyleSheet("background: gray");
    ui->checkBox_2->setStyleSheet("background: gray");
    ui->checkBox_3->setStyleSheet("background: gray");
    ui->checkBox_4->setStyleSheet("background: gray");
    ui->lineEditWmPath->setStyleSheet("background: gray");
    ui->pushButtonWmReset->setStyleSheet("background: gray");
    ui->pushButtonWmSet->setStyleSheet("background: gray");
    ui->line->setStyleSheet("background: gray");
    ui->line_2->setStyleSheet("background: gray");
    ui->line_3->setStyleSheet("background: gray");
    ui->line_4->setStyleSheet("background: gray");
    ui->lineEditWmPath->setStyleSheet("background: gray");
    ui->label_2BigPic->setStyleSheet("color: gray");
    ui->widgetInfo->hide();
}

Inspector::~Inspector()
{
    delete ui;
}

void Inspector::resetWm2BigPic()
{
    ui->label_2BigPic->setText("The image you are trying to use is too big.\nScale it down first and try again.");
    ui->widgetInfo->show();
    ui->lineEditWmPath->clear();
    mFileName.clear();
    emit wmResetPath(true);
}

void Inspector::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    //background
    painter.fillRect(this->rect(), QColor::fromRgb(20, 20, 20, 255));

    //main background
    QRadialGradient gradient(70, height() - 10, width(), width() - 10, 20);
        gradient.setColorAt(1, QColor::fromRgbF(0.15, 0.15, 0.15, 1));
        gradient.setColorAt(0, QColor::fromRgbF(0.3, 0.3, 0.3, 0.8));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(5, 5, width() - 10, height() - 10, 5.0, 5.0);
}

void Inspector::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void Inspector::dropEvent(QDropEvent *event)
{
    if(ui->widgetWaterMark->geometry().contains(event->pos()))
    {
        foreach (const QUrl &url, event->mimeData()->urls())
        {
            mFileName = url.toLocalFile();
            ui->lineEditWmPath->setText(mFileName);
        }
    }
}

void Inspector::on_horizontalSlider_Br_valueChanged(int value)
{
    emit brightValue(value);
    QString temp = "Value: " + QString::number(value);
    ui->label_bright->setText(temp);

    sample_palette.setColor(QPalette::WindowText, Qt::white);
    ui->label_bright->setPalette(sample_palette);
}

void Inspector::on_horizontalSlider_R_valueChanged(int value)
{
    emit redValue(value);
    QString temp = "Value: " + QString::number(value);
    ui->label_2->setText(temp);
}

void Inspector::on_horizontalSlider_G_valueChanged(int value)
{
    emit greenValue(value);
    QString temp = "Value: " + QString::number(value);
    ui->label_3->setText(temp);
}

void Inspector::on_horizontalSlider_B_valueChanged(int value)
{
    emit blueValue(value);
    QString temp = "Value: " + QString::number(value);
    ui->label_4->setText(temp);
}

//Brightness check box
void Inspector::on_checkBox_clicked()
{
    mIfBright = !mIfBright;
    emit ifBrightBool(mIfBright);
}

//Red channel check box
void Inspector::on_checkBox_2_clicked()
{
    mIfRed = !mIfRed;
    emit ifRedChannel(mIfRed);
}

//Green channel check box
void Inspector::on_checkBox_3_clicked()
{
    mIfGreen = !mIfGreen;
    emit ifGreenChannel(mIfGreen);
}

//Blue channel check box
void Inspector::on_checkBox_4_clicked()
{
    mIfBlue = !mIfBlue;
    emit ifBlueChannel(mIfBlue);
}

void Inspector::on_horizontalSlider_wm_opacity_valueChanged(int value)
{
    QString temp = QString::number(value) + "%";
    ui->label_wm_opacityvalue->setText(temp);
    emit wmOpacityValue((double)value / 100);
}

void Inspector::on_horizontalSlider_wmSize_valueChanged(int value)
{
    QString temp = QString::number(value) + "%";
    ui->label_wmSizeValue->setText(temp);
    emit wmScaleValue(value);
}

void Inspector::on_horizontalSlider_Br_sliderPressed()
{
    ui->horizontalSlider_Br->setCursor(Qt::ClosedHandCursor);
}

void Inspector::on_horizontalSlider_Br_sliderReleased()
{
    ui->horizontalSlider_Br->setCursor(Qt::OpenHandCursor);
}

void Inspector::on_horizontalSlider_R_sliderPressed()
{
    ui->horizontalSlider_R->setCursor(Qt::ClosedHandCursor);
}

void Inspector::on_horizontalSlider_R_sliderReleased()
{
    ui->horizontalSlider_R->setCursor(Qt::OpenHandCursor);
}

void Inspector::on_horizontalSlider_G_sliderPressed()
{
    ui->horizontalSlider_G->setCursor(Qt::ClosedHandCursor);
}

void Inspector::on_horizontalSlider_G_sliderReleased()
{
    ui->horizontalSlider_G->setCursor(Qt::OpenHandCursor);
}

void Inspector::on_horizontalSlider_B_sliderPressed()
{
    ui->horizontalSlider_B->setCursor(Qt::ClosedHandCursor);
}

void Inspector::on_horizontalSlider_B_sliderReleased()
{
    ui->horizontalSlider_B->setCursor(Qt::OpenHandCursor);
}

void Inspector::on_horizontalSlider_wm_opacity_sliderPressed()
{
    ui->horizontalSlider_wm_opacity->setCursor(Qt::ClosedHandCursor);
}

void Inspector::on_horizontalSlider_wm_opacity_sliderReleased()
{
    ui->horizontalSlider_wm_opacity->setCursor(Qt::OpenHandCursor);
}

void Inspector::on_horizontalSlider_wmSize_sliderPressed()
{
     ui->horizontalSlider_wmSize->setCursor(Qt::ClosedHandCursor);
}

void Inspector::on_horizontalSlider_wmSize_sliderReleased()
{
     ui->horizontalSlider_wmSize->setCursor(Qt::OpenHandCursor);
}

void Inspector::on_spinBox_X_valueChanged(int arg1)
{
    emit wmPosX(arg1);
}

void Inspector::on_spinBox_Y_valueChanged(int arg1)
{
    emit wmPosY(arg1);
}

void Inspector::on_pushButtonWmReset_clicked()
{
    ui->widgetInfo->hide();
    ui->lineEditWmPath->clear();
    mFileName.clear();
    emit wmResetPath(true);
}

void Inspector::on_pushButtonWmSet_clicked()
{
//    ui->widgetInfo->hide();
    if(!mFileName.isEmpty())
    emit wmSetPressed(mFileName);
}



