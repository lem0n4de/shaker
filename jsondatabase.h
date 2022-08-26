#ifndef JSONDATABASE_H
#define JSONDATABASE_H

#include <QException>
#include <QPointer>
#include <QStandardPaths>
#include <lesson.h>

class JsonDatabase
{
    public:
        static QString database_dir() { return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation); }
        static QString database_name() { return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/lessons.json"; }
        static QString default_json_name() { return ":/lessons.json"; }
        static void save_lessons(const QList<QPointer<Lesson>>& lessons);
        static QList<QPointer<Lesson>> retrieve_lessons();

    signals:
};

#endif // JSONDATABASE_H
