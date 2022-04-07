#ifndef LESSON_H
#define LESSON_H

#include <QObject>
#include <video.h>

class Lesson : public QObject
{
        Q_OBJECT
    public:
        QString teacher;
        QString name;
        std::vector<Video*> videos;
        Lesson(const QString id, const QString name, const QString teacher);
    signals:

    private:
        QString id;
};

#endif // LESSON_H
