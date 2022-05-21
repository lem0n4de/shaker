//
// Created by lem0n on 21/05/2022.
//

#ifndef SHAKER_LOGHANDLER_H
#define SHAKER_LOGHANDLER_H

#include <QtMessageHandler>
#include <QDateTime>
#include <QFile>
#include <QDir>

void shaker_log_handler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QString date = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    QString txt = "";

    switch(type) {
        case QtDebugMsg:
            txt += QString("[DEBUG] [%1] %2 %3 %4").arg(date, file, function, msg);
            break;
        case QtWarningMsg:
            txt += QString("[WARNING] [%1] %2 %3 %4").arg(date, file, function, msg);
            break;
        case QtCriticalMsg:
            txt += QString("[CRITICAL] [%1] %2 %3 %4").arg(date, file, function, msg);
            break;
        case QtFatalMsg:
            txt += QString("[FATAL] [%1] %2 %3 %4").arg(date, file, function, msg);
            break;
        case QtInfoMsg:
            txt += QString("[INFO] [%1] %2 %3 %4").arg(date, file, function, msg);
            break;
    }

#ifdef SHAKER_DEBUG
    fprintf(stderr, "\n%s", txt.toLatin1().data());
#else
    if (type != QtDebugMsg) {
        fprintf(stderr, "\n%s", txt.toLatin1().data());
    }
#endif

    QDir dir("log");
    if (!dir.exists()) {
        auto rv = dir.mkpath(dir.absolutePath());
        if (!rv) {
            qDebug() << "log directory couldn't be created";
            return;
        }
    }
    QString log_file_name = QString("log/log-%1").arg(QDate::currentDate().toString("dd-MM-yyyy"));
    QFile out(log_file_name);
    if (out.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream text_stream(&out);
        text_stream << txt << Qt::endl;
    } else {
        qDebug() << "log file couldn't be opened";
    }
}


#endif //SHAKER_LOGHANDLER_H
