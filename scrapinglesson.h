#ifndef SCRAPINGLESSON_H
#define SCRAPINGLESSON_H

#include <QString>

class ScrapingLesson
{
    public:
        ScrapingLesson(const QString name, const QString html_id);
        ScrapingLesson(const QString name, const QString html_id, const QString teacher, const QString teacher_html_id);
        QString html_id;
        QString name;
        QString teacher;
        QString teacher_html_id;
};

#endif // SCRAPINGLESSON_H
