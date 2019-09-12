#include "saveframesdialog.hpp"
#include "ui_saveframesdialog.h"
#include <QDebug>

SaveFramesDialog::SaveFramesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveFramesDialog)
{
    ui->setupUi(this);
    ui->groupBox->setStyleSheet("color: white");
    ui->spinBox->setStyleSheet("background: gray");
    ui->buttonBox->setStyleSheet("background: gray; color: white");
    ui->spinBox->setRange(0, 1000);
}

SaveFramesDialog::~SaveFramesDialog()
{
    delete ui;
}

QString SaveFramesDialog::exportPath(QString fileName)
{
    mExportPath =  QFileDialog::getSaveFileName(this, "Save file", fileName, tr("Images (*.png *.xpm *.jpg);; TIFF (*.tiff);; All files (*.*)"));
    return mExportPath;
}

void SaveFramesDialog::paintEvent(QPaintEvent *e)
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

int SaveFramesDialog::numFramesToSave() const
{
    return ui->spinBox->value();
}

void SaveFramesDialog::accept()
{
    QDialog::accept();
}

void SaveFramesDialog::reject()
{
    QDialog::reject();
}
