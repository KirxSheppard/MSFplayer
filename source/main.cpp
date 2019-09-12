#include "mainwindow.h"
#include "initialdialog.h"
#include <QFileOpenEvent>

const QString msfLogo  = ":/resources/ikonamsf.png";
QString videoInPutPath;
int numOfFrames;
double startFromTimeCode;

class MyApplication : public QApplication
{
    Q_OBJECT

public:
    MyApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen)
        {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            emit newFile(openEvent->file());
        }
        return QApplication::event(event);
    }

signals:
    void newFile(const QString &filePath);
};

int main(int argc, char *argv[])
{
   qInstallMessageHandler([](QtMsgType t , const QMessageLogContext &c, const QString &s) {
        cerr << c.line << ": " << qUtf8Printable(qFormatLogMessage(t, c, s)) << endl;
   });

   MyApplication a(argc, argv);

#ifndef Q_OS_WINDOWS
    a.setStyle("Fusion");
#endif

   InitialDialog initDial;
   initDial.setWindowTitle("Import video");

   QObject::connect(&a, &MyApplication::newFile,
                    &initDial, &InitialDialog::setFileName);

   if (argc > 1)
       initDial.setFileName(argv[1]);

   initDial.setFileName("/Users/kamil/Desktop/Target Earth - Festiwal.mp4");//temp
   if (initDial.exec() == QDialog::Accepted)
   {
        videoInPutPath = initDial.getInputFilePath();
        numOfFrames = initDial.numOfFrames();
        startFromTimeCode = initDial.initTimeCode();
   }
   else return 0;

   MainWindow w(msfLogo);
   w.setWindowTitle("MSF Player");
   w.videoPlayerInit(videoInPutPath, numOfFrames, startFromTimeCode);
   w.show();

   return a.exec();
}

#include <main.moc>
