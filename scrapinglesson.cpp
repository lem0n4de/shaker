#include "scrapinglesson.h"


ScrapingLesson::ScrapingLesson(const QString name, const QString html_id) : name(name), html_id(html_id) {}

ScrapingLesson::ScrapingLesson(const QString name, const QString html_id, const QString teacher, const QString teacher_html_id)
    : teacher_html_id(teacher_html_id), html_id(html_id), name(name), teacher(teacher) {}
