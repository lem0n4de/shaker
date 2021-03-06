#ifndef LESSON_H
#define LESSON_H

#include <QObject>
#include <QDebug>
#include <QList>
#include <QPointer>
#include <functional>
#include <video.h>

class Lesson : public QObject
{
        Q_OBJECT
    public:
        QString id;
        QString teacher;
        QString name;
        QList<QPointer<Video>> videos;
        Lesson(QString id, QString name, QString teacher);
        Lesson(QString name, QString teacher);
        static QList<QPointer<Lesson>> filter_by_name(const QList<QPointer<Lesson>>& lessons, const QString& name);
        static QList<QPointer<Lesson>> filter_by_teacher(const QList<QPointer<Lesson>>& lessons, const QString& teacher);
    signals:

    private:
};

QDebug inline operator<<(QDebug d, const QPointer<Lesson>& f) {
    QDebug nsp = d.nospace();
    nsp << "Lesson(name=" << f->name << ", teacher=" << f->teacher << ", video_size=" << f->videos.size();
    nsp << "\n";
    return nsp;
}

#endif // LESSON_H
