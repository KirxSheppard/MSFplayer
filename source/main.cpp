#include "mainwindow.h"

const QString msfLogo  = "/Volumes/SANDISK/ikona msf small.png";

int main(int argc, char *argv[])
{
   qInstallMessageHandler([](QtMsgType t , const QMessageLogContext &c, const QString &s) {
        cerr << c.line << ": " << qUtf8Printable(qFormatLogMessage(t, c, s)) << endl;
   });

   QApplication a(argc, argv);
   a.setStyle("Fusion");

   MainWindow w(msfLogo);
   w.setWindowTitle("MSF FFmpeg player");
   w.importVideo();

   //Needs some more work, on init abort doesn't close the app
   if(w.checkIfInitAborded())
   {
       qDebug()<<"here";
       a.exec(); //even this doesn't work
//       w.close();
//       QApplication::quit();
//       qApp->exit();
   }
   else w.show();

   return a.exec();
}
