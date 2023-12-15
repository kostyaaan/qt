#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QFile file(":/styles/styles.css");
    file.open(QFile::ReadOnly);
    QString strCSS = QLatin1String(file.readAll());
    qApp->setStyleSheet(strCSS);

    QTranslator translator;
    QString str = QCoreApplication::applicationDirPath() + "/QReader_en"; translator.load(str);
    a.installTranslator(&translator);

    w.show();
    return a.exec();
}
