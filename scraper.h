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
        void scrape();
        ~Scraper() override;
        [[nodiscard]] bool is_scraping() const;

    public slots:
        void loading_finished();

    signals:
        void new_video_scraped(QPointer<Video> video);
        void hc_atf_found();
        void hc_atf_not_found();
        void start_video_scrape_of_hc_atf_lesson();
        void start_video_scrape_of_non_hc_atf_lesson();
        void start_scrape_of_next_lesson();

    protected:
        void scrape_anasayfa();
        void scrape_video_kategori();
        void scrape_video_grup_dersleri();


    private slots:
        void _on_hc_atf_found();
        void _on_hc_atf_not_found();
        void _on_start_video_scrape_of_hc_atf_lesson();
        void _on_start_video_scrape_of_non_hc_atf_lesson();
        void _on_start_scrape_of_next_lesson();

    private:
        Ui::Scraper *ui;
        QWebEngineProfile* profile = nullptr;
        QWebEnginePage* page = nullptr;
        QList<std::pair<QString, QString>> lessons_to_scrape; // id, text without teachers
        QList<TeacherLesson> teacher_lessons; // lessons with teachers
        std::pair<QString, QString> searching_lesson_id_and_title; // id, text without teacher
        bool lesson_names_scraped = false;
        bool _working = false;

        typedef void (Scraper::*video_scraper)();
        video_scraper ongoing_video_scraping_function = nullptr;

        std::pair<TeacherLesson, QPointer<Lesson>> current_lesson;
        QList<QPointer<Lesson>> finished_lessons;
        bool ongoing_video_scraping = false;

        void get_lesson_names();
        void get_video_names_for_current_lesson();
        void scrape_video_of_hc_atf_lesson_and_click_next_lesson();
        void scrape_video_of_non_hc_atf_lesson_and_click_next_lesson();
        void click_element_by_id(const QString& id);
        QList<TeacherLesson> build_remaining_lesson_list();
        template<typename Functor, typename OnError>
        void wait_for_element_to_appear(const QString& selector, Functor callback, OnError on_error, unsigned int timeout = 30);
        template<typename Functor>
        void wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout = 30);

        void closeEvent(QCloseEvent *event) override;
};

#endif // SCRAPER_H
