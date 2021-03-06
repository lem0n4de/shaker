#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <loghandler.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("shaker");
    QApplication::setOrganizationName("shaker");
    qInstallMessageHandler(shaker_log_handler);
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "shaker_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
