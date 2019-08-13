#include "initialdialog.h"
#include "ui_initialdialog.h"
#include <QDebug>

InitialDialog::InitialDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitialDialog)
{
    setAcceptDrops(true);
    ui->setupUi(this);
    ui->spinBoxFrames->setRange(0,500);
}

InitialDialog::~InitialDialog()
{
    delete ui;
}

void InitialDialog::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void InitialDialog::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls())
    {
        QString fileName = url.toLocalFile();
        ui->lineEditVideoPath->setText(fileName);
    }
}

int InitialDialog::numOfFrames() const
{
    return ui->spinBoxFrames->value();
}

double InitialDialog::initTimeCode() const
{
    return ui->SpinBoxTimeCode->value();
}

QString InitialDialog::getInputFileName() const
{
    return ui->lineEditVideoPath->text();
}

void InitialDialog::on_pushButton_pressed() //do dopisania
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open file");

    //temp
    ui->lineEditVideoPath->setText(fileName);
}

void InitialDialog::on_buttonBox_accepted()
{
    if(ui->lineEditVideoPath->text() != "")
    {
        QDialog::accept();
    }
    else
    {
        QMessageBox::warning(this, "Error", "Missing video path");
    }
}

void InitialDialog::on_buttonBox_rejected()
{
    QDialog::rejected();
}
