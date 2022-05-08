#include "video.h"
#include <uuid.h>

Video::Video(const QString id, const QString name, const QString teacher, const QString url)
{
    this->id = id;
    this->name = name;
    this->teacher = teacher;
    this->url = url;
}

Video::Video(const QString name, const QString teacher, const QString url)
    : name(name), teacher(teacher), url(url)
{
    this->id = QString::fromStdString(uuid::generate_uuid_v4());
}

bool Video::operator==(const Video& rhs)
{
    return this->id == rhs.id || this->name == rhs.name;
}
