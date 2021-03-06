#ifndef TEACHERLESSON_H
#define TEACHERLESSON_H

#include <QString>
#include <QDebug>
#include <QPointer>

class TeacherLesson
{
    public:
        TeacherLesson();
        TeacherLesson(QString name, QString html_id);
        TeacherLesson(QString name, QString html_id, QString teacher, QString teacher_html_id);
        QString html_id;
        QString name;
        QString teacher;
        QString teacher_html_id;

        void start();
        [[nodiscard]] bool is_started() const;
        [[nodiscard]] bool is_ended() const;
        void end();

        struct video_info {
                QString id;
                QString name;
        };
        video_info pop_video_info();
        QList<video_info> video_infos;
    private:
        bool started = false;
        bool ended = false;
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
