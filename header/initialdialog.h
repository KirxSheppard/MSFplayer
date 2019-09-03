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
    bool dialAccepted();
    QString getInputFilePath() const;

    void setFileName(const QString &fileName);

private slots:
    void on_pushButton_pressed();

    void on_pushButton_advanced_pressed();

    void on_pushButton_cancel_clicked();

    void on_pushButton_import_clicked();

private:
    bool mIfAdvanced = false;
    bool dialacp = false;
    Ui::InitialDialog *ui;
};

#endif // INITIALDIALOG_H
