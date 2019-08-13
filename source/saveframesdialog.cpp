#include "saveframesdialog.hpp"
#include "ui_saveframesdialog.h"

SaveFramesDialog::SaveFramesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveFramesDialog)
{
    ui->setupUi(this);


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
//    QPainter painter(this);
    //background
    //    painter.fillRect(this->rect(),QColor::fromRgb(100, 100, 100, 255));
}

int SaveFramesDialog::numFramesToSave() const
{
    return ui->spinBox->value() + 1;
}

void SaveFramesDialog::accept()
{
    QDialog::accept();
}

void SaveFramesDialog::reject()
{
    QDialog::reject();
}
