#ifndef SAVEFRAMESDIALOG_HPP
#define SAVEFRAMESDIALOG_HPP

#include <QDialog>
#include <QFileDialog>
#include <QPainter>

namespace Ui {
class SaveFramesDialog;
}

class SaveFramesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveFramesDialog(QWidget *parent = nullptr);
    ~SaveFramesDialog();
    QString exportPath(QString fileName);
    void paintEvent(QPaintEvent *e) override;

    int numFramesToSave() const;

private slots:
    void accept() override;
    void reject() override;

private:
    Ui::SaveFramesDialog *ui;
    QString mExportPath;
};

#endif // SAVEFRAMESDIALOG_HPP
