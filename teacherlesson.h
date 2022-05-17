#ifndef TEACHERLESSON_H
#define TEACHERLESSON_H

#include <QString>
#include <QDebug>
#include <QPointer>

class TeacherLesson
{
    public:
        TeacherLesson();
        TeacherLesson(const QString name, const QString html_id);
        TeacherLesson(const QString name, const QString html_id, const QString teacher, const QString teacher_html_id);
        QString html_id;
        QString name;
        QString teacher;
        QString teacher_html_id;
        bool done;

        struct video_info {
                QString id;
                QString name;
        };
        QList<video_info> video_infos;
};

QDebug inline operator<<(QDebug d, const TeacherLesson v) {
    QDebug nsp = d.nospace();
    nsp << "Lesson(name=" << v.name
        << ", teacher=" << v.teacher
        << ", html_id=" << v.html_id
        << ", teacher_html_id=" << v.teacher_html_id
        << ", video_count=" << v.video_infos.size()
        << ")";
    return nsp;
}

#endif // TEACHERLESSON_H
