#include "video.h"

Video::Video(const QString id, const QString name, const QString teacher, const QString url)
{
    this->id = id;
    this->name = name;
    this->teacher = teacher;
    this->url = url;
}