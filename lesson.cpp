#include "lesson.h"
#include <uuid.h>

#include <utility>

Lesson::Lesson(QString id, QString name, QString teacher)
        : id(std::move(id)), teacher(std::move(teacher)), name(std::move(name))
{
}

Lesson::Lesson(QString name, QString teacher)
        : teacher(std::move(teacher)), name(std::move(name))
{
    this->id = QString::fromStdString(uuid::generate_uuid_v4());
}

QList<QPointer<Lesson>> Lesson::filter_by_name(const QList<QPointer<Lesson>> &lessons, const QString &name)
{
    QList<QPointer<Lesson>> v;
    for (const auto &lesson: lessons)
    {
        if (lesson->name == name) v.push_back(lesson);
    }
    return v;
}

QList<QPointer<Lesson>> Lesson::filter_by_teacher(const QList<QPointer<Lesson>> &lessons, const QString &teacher)
{
    QList<QPointer<Lesson>> v;
    for (const auto &lesson: lessons)
    {
        if (lesson->teacher == teacher) v.push_back(lesson);
    }
    return v;
}
