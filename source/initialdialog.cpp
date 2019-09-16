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
    ui->groupBox->setStyleSheet("color: white");
    ui->pushButton_cancel->setStyleSheet("background: gray; color: white");
    ui->pushButton_import->setStyleSheet("background: gray; color: white");
    ui->spinBoxFrames->setStyleSheet("background: gray");
    ui->SpinBoxTimeCode->setStyleSheet("background: gray");
    ui->lineEditVideoPath->setStyleSheet("background: gray");
    ui->pushButton->setStyleSheet("background: gray");
    ui->widgetFrames->hide();
    ui->widgetTimeCode->hide();
    ui->spinBoxFrames->setRange(0,500);
    ui->pushButton_advanced->setStyleSheet("background-color: transparent; border: none;");
    ui->widgetAdvanced->setStyleSheet("font: italic;");
    resize(400,100);
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

void InitialDialog::paintEvent(QPaintEvent *event)
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

void InitialDialog::setFileName(const QString &fileName)
{
    ui->lineEditVideoPath->setText(fileName);
}

void InitialDialog::on_pushButton_pressed()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open file");
    if (!fileName.isEmpty())
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
    QDialog::reject();
}

void InitialDialog::on_pushButton_import_clicked()
{
    if(!ui->lineEditVideoPath->text().isEmpty())
    {
        dialacp = true;
        QDialog::accept();
    }
    else
    {
        dialacp = false;
        QMessageBox::warning(this, "Error", "Missing video path");
    }
}
