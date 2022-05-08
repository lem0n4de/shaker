#include "lesson.h"
#include <uuid.h>

Lesson::Lesson(const QString id, const QString name, const QString teacher)
{
    this->id = id;
    this->name = name;
    this->teacher = teacher;
}

Lesson::Lesson(const QString name, const QString teacher)
    :name(name), teacher(teacher)
{
    this->id = QString::fromStdString(uuid::generate_uuid_v4());
}

QList<QPointer<Lesson>> Lesson::filter_by_name(QList<QPointer<Lesson>> lessons, QString name)
{
    QList<QPointer<Lesson>> v;
    for (auto lesson: lessons) {
        if (lesson->name == name) v.push_back(lesson);
    }
    return v;
}

QList<QPointer<Lesson>> Lesson::filter_by_teacher(QList<QPointer<Lesson>> lessons, QString teacher)
{
    QList<QPointer<Lesson>> v;
    for (auto lesson: lessons) {
        if (lesson->teacher == teacher) v.push_back(lesson);
    }
    return v;
}
