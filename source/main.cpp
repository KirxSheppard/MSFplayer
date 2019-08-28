#include "mainwindow.h"
#include "initialdialog.h"


const QString msfLogo  = "/Users/kamil/Desktop/Studia/VideoDecodeFFMPEG/resources/ikonamsf.png";
QString videoInPutPath;
int numOfFrames;
double startFromTimeCode;

int main(int argc, char *argv[])
{
   qInstallMessageHandler([](QtMsgType t , const QMessageLogContext &c, const QString &s) {
        cerr << c.line << ": " << qUtf8Printable(qFormatLogMessage(t, c, s)) << endl;
   });

   QApplication a(argc, argv);
   a.setStyle("Fusion");

   InitialDialog initDial;
   initDial.setWindowTitle("Import video");

   if (initDial.exec() == QDialog::Accepted)
   {
        videoInPutPath = initDial.getInputFilePath();
        numOfFrames = initDial.numOfFrames();
        startFromTimeCode = initDial.initTimeCode();
   }
   else return 0;

   MainWindow w(msfLogo);
   w.setWindowTitle("MSF FFmpeg player");
   w.videoPlayer(videoInPutPath, numOfFrames, startFromTimeCode);
   w.show();

   return a.exec();
}
