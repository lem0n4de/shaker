#ifndef VIDEO_H
#define VIDEO_H

#include <QObject>
#include <QPointer>
#include <QDebug>

class Video : public QObject
{
        Q_OBJECT
    public:
        Video(QString id, QString name, QString teacher, QString url);
        Video(QString name, QString teacher, QString url);
        QString name;
        QString teacher;
        QString url;
        QString id;
        QString lesson_name;
        bool operator==(Video const& rhs) const;

    signals:
};

QDebug inline operator<<(QDebug d, const QPointer<Video> v) {
    QDebug nsp = d.nospace();
    nsp << "Video(id=" << v->id
        << ", name=" << v->name
        << ", lesson_name=" << v->lesson_name
        << ", teacher=" << v->teacher
        << ", url=" << v->url
        << ")";
    return nsp;
}


#endif // VIDEO_H
