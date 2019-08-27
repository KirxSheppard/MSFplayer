#include "initialdialog.h"
#include "ui_initialdialog.h"
#include <QDebug>
#include <QTimer>

InitialDialog::InitialDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitialDialog)
{
    setAcceptDrops(true);
    ui->setupUi(this);
    ui->widgetFrames->hide(); //temp
    ui->widgetTimeCode->hide();
    ui->spinBoxFrames->setRange(0,500);
    ui->pushButton_advanced->setStyleSheet("background-color: transparent; border: none;");
    ui->widgetAdvanced->setStyleSheet("font: italic;");
    resize(400,100);
//    QDialog::setResult(2);
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

bool InitialDialog::dialAccepted()
{
    return dialacp;
}

QString InitialDialog::getInputFilePath() const
{
    return ui->lineEditVideoPath->text();
}

void InitialDialog::on_pushButton_pressed() //do dopisania
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open file");
    ui->lineEditVideoPath->setText(fileName);
}

void InitialDialog::on_pushButton_advanced_pressed()
{
    mIfAdvanced = !mIfAdvanced;
    if(mIfAdvanced)
    {
        ui->widgetFrames->show();
        ui->widgetTimeCode->show();
    }
    else
    {
        ui->widgetFrames->hide();
        ui->widgetTimeCode->hide();
        QTimer::singleShot(0, this, [this] {
            QTimer::singleShot(0, this, [this] {
                resize(width(), 1);
            });
        });
    }
    ui->pushButton_advanced->setText(mIfAdvanced ? "- Advanced settings" : "+ Advanced settings");
}

void InitialDialog::on_pushButton_cancel_clicked()
{
    qDebug()<<"cancel";
//     QDialog::setResult(0);
//    dialClosed = true;
//    emit ifClosedChecked(dialClosed);
    QDialog::reject();
    qApp->exit();
//    close();
}

void InitialDialog::on_pushButton_import_clicked()
{
    if(!ui->lineEditVideoPath->text().isEmpty())
    {
        dialacp = true;
//        QDialog::setResult(1);
        QDialog::accept();
        qDebug()<< result();
    }
    else
    {
        dialacp = false;
        QMessageBox::warning(this, "Error", "Missing video path");
    }
}
