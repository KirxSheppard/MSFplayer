#ifndef INITIALDIALOG_H
#define INITIALDIALOG_H

#include <QDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QFileDialog>

namespace Ui {
class InitialDialog;
}

class InitialDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InitialDialog(QWidget *parent = nullptr);
    ~InitialDialog();

    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    int numOfFrames() const;
    double initTimeCode() const;
    QString getInputFileName() const;

private slots:
    void on_pushButton_pressed();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_pushButton_advanced_pressed();


private:
    bool mIfAdvanced = false;
    Ui::InitialDialog *ui;
};

#endif // INITIALDIALOG_H
