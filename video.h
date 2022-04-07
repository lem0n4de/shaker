#ifndef VIDEO_H
#define VIDEO_H

#include <QObject>

class Video : public QObject
{
        Q_OBJECT
    public:
        Video(const QString id, const QString name, const QString teacher, const QString url);
        QString name;
        QString teacher;
        QString url;
        QString id;

    signals:
};

#endif // VIDEO_H
