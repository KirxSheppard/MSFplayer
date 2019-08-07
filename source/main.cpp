#include "mainwindow.h"

//const int numOfFrames = 200;
//const int desiredPos = 40;
const QString videoInPut = "E:/Episode 2.m4v"; //8-bit video, in the future i'll allow user to choose any file he wants
const QString msfLogo  = "E:/ikona msf small.png";

QString fileName()
{
    //Gets the video file name
    int pos = videoInPut.length() - videoInPut.lastIndexOf('/') - 1;
    QString fileName = videoInPut.right(pos);
    return fileName;
}

int main(int argc, char *argv[])
{
   qInstallMessageHandler([](QtMsgType t , const QMessageLogContext &c, const QString &s) {
        cerr << c.line << ": " << qUtf8Printable(qFormatLogMessage(t, c, s)) << endl;
   });

   QApplication a(argc, argv);

   MainWindow w(fileName(), videoInPut, msfLogo);
   w.setWindowTitle("MSF FFmpeg player");
   w.videoPlayer();
   w.show();

   return a.exec();
}
