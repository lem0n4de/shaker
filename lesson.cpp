#include "lesson.h"

Lesson::Lesson(const QString id, const QString name, const QString teacher) {
  this->id = id;
  this->name = name;
    this->teacher = teacher;
}

std::vector<Lesson*> Lesson::filter_by_name(std::vector<Lesson*> lessons, QString name)
{
    std::vector<Lesson*> v;
    for (auto lesson: lessons) {
        if (lesson->name == name) v.push_back(lesson);
    }
    return v;
}

std::vector<Lesson*> Lesson::filter_by_teacher(std::vector<Lesson*> lessons, QString teacher)
{
    std::vector<Lesson*> v;
    for (auto lesson: lessons) {
        if (lesson->teacher == teacher) v.push_back(lesson);
    }
    return v;
}
