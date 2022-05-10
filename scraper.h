#ifndef SCRAPER_H
#define SCRAPER_H

#include <QMainWindow>
#include <QUrl>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QPointer>
#include <teacherlesson.h>
#include <lesson.h>

namespace Ui {
    class Scraper;
}

class Scraper : public QMainWindow
{
        Q_OBJECT

    public:
        explicit Scraper(QWidget *parent = nullptr);
        ~Scraper();

    public slots:
        void loading_finished();
        void _on_hc_atf_found();
        void _on_hc_atf_not_found();
        void _on_start_video_scrape_of_hc_atf_lesson();
        void _on_start_video_scrape_of_non_hc_atf_lesson();
        void _on_start_scrape_of_next_lesson();
    signals:
        void hc_atf_found();
        void hc_atf_not_found();
        void start_video_scrape_of_hc_atf_lesson();
        void start_video_scrape_of_non_hc_atf_lesson();
        void start_scrape_of_next_lesson();

    private:
        Ui::Scraper *ui;
        QWebEngineProfile* profile;
        QWebEnginePage* page;
        QList<std::pair<QString, QString>> lessons_to_scrape; // id, text without teachers
        QList<TeacherLesson> teacher_lessons; // lessons with teachers
        std::pair<QString, QString> searching_lesson_id_and_title; // id, text without teacher
        bool lesson_names_scraped = false;

        typedef void (Scraper::*video_scraper)(void);
        video_scraper ongoing_video_scraping_function;

        std::pair<TeacherLesson, QPointer<Lesson>> current_lesson;
        QList<TeacherLesson> teacher_lessons_with_video_info;
        QList<QPointer<Lesson>> finished_lessons;
        bool ongoing_video_scraping = false;

        void scrape_anasayfa();
        void scrape_video_kategori();
        void scrape_video_grup_dersleri();
        void get_lesson_names();
        void get_video_names_for_current_lesson();
        void scrape_video_of_hc_atf_lesson_and_click_next_lesson();
        void scrape_video_of_non_hc_atf_lesson_and_click_next_lesson();
        void click_element_by_id(QString id);
        QList<TeacherLesson> build_remaining_lesson_list();
        template<typename Functor, typename OnError>
        void wait_for_element_to_appear(QString selector, Functor callback, OnError on_error, unsigned int timeout = 30);
        template<typename Functor>
        void wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout = 30);
};

#endif // SCRAPER_H
