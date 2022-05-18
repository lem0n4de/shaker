#include "teacherlesson.h"


TeacherLesson::TeacherLesson()
    :name(""), html_id(""), teacher(""), teacher_html_id("") {}

TeacherLesson::TeacherLesson(const QString name, const QString html_id) : name(name), html_id(html_id) {}

TeacherLesson::TeacherLesson(const QString name, const QString html_id, const QString teacher, const QString teacher_html_id)
    : teacher_html_id(teacher_html_id), html_id(html_id), name(name), teacher(teacher) {}

bool TeacherLesson::finished()
{
    qDebug() << "STARTED == " << this->started
             << "ENDED == " << this->ended
             << "VIDEOS_INFO_EMPTY == " << this->video_infos.empty();
    return this->started && this->ended && this->video_infos.empty();
}

void TeacherLesson::start()
{
    this->started = true;
}

bool TeacherLesson::is_started()
{
    return this->started;
}

bool TeacherLesson::is_ended()
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
