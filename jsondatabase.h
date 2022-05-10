#ifndef JSONDATABASE_H
#define JSONDATABASE_H

#include <QException>
#include <QPointer>
#include <QStandardPaths>
#include <lesson.h>

class JsonDatabase
{
    public:
        inline static const QString database_name = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/lessons.json";
        static void save_lessons(QList<QPointer<Lesson>> lessons);
        static QList<QPointer<Lesson>> retrieve_lessons();

    signals:
};

#endif // JSONDATABASE_H
