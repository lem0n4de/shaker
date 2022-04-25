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
        QString teacher;
        QString name;
        QList<QPointer<Video>> videos;
        Lesson(const QString id, const QString name, const QString teacher);
        static QList<QPointer<Lesson>> filter_by_name(QList<QPointer<Lesson>> lessons, QString name);
        static QList<QPointer<Lesson>> filter_by_teacher(QList<QPointer<Lesson>> lessons, QString teacher);
    signals:

    private:
        QString id;
};

QDebug inline operator<<(QDebug d, const QPointer<Lesson> f) {
    QDebug nsp = d.nospace();
    nsp << "Lesson(name=" << f->name << ", teacher=" << f->teacher << ", video_size=" << f->videos.size();
    nsp << "\n";
    return nsp;
}

#endif // LESSON_H
