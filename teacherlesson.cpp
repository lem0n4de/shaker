#include "teacherlesson.h"

#include <utility>


TeacherLesson::TeacherLesson()
        : html_id(""), name(""), teacher(""), teacher_html_id("") {}

TeacherLesson::TeacherLesson(QString name, QString html_id)
        : html_id(std::move(html_id)), name(std::move(name)) {}

TeacherLesson::TeacherLesson(QString name, QString html_id, QString teacher, QString teacher_html_id)
        : html_id(std::move(html_id)), name(std::move(name)), teacher(std::move(teacher)),
          teacher_html_id(std::move(teacher_html_id)) {}

void TeacherLesson::start()
{
    this->started = true;
}

bool TeacherLesson::is_started() const
{
    return this->started;
}

bool TeacherLesson::is_ended() const
{
    return this->ended;
}

void TeacherLesson::end()
{
    this->ended = true;
}

TeacherLesson::video_info TeacherLesson::pop_video_info()
{
    auto info = this->video_infos[0];
    this->video_infos.pop_front();
    return info;
}
