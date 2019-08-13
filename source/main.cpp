#include "mainwindow.h"

const QString videoInPut = "/Volumes/SANDISK/Episode 2.m4v"; //8-bit video, in the future i'll allow user to choose any file he wants
const QString msfLogo  = "/Volumes/SANDISK/ikona msf small.png";

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
   a.setStyle("Fusion");

   MainWindow w(fileName(), videoInPut, msfLogo);
   w.setWindowTitle("MSF FFmpeg player");
   w.videoPlayer();
   w.show();

   return a.exec();
}
