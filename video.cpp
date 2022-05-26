#include "video.h"
#include <uuid.h>

#include <utility>

Video::Video(QString id, QString name, QString teacher, QString url)
        : name(std::move(name)), teacher(std::move(teacher)), url(std::move(url)), id(std::move(id)) {}

Video::Video(QString name, QString teacher, QString url)
        : name(std::move(name)), teacher(std::move(teacher)), url(std::move(url))
{
    this->id = QString::fromStdString(uuid::generate_uuid_v4());
}

bool Video::operator==(const Video &rhs) const
{
    return this->id == rhs.id || this->name == rhs.name;
}
