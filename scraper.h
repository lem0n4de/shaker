#ifndef SCRAPER_H
#define SCRAPER_H

#include <QMainWindow>
#include <QUrl>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <scrapinglesson.h>

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
        void scrape_anasayfa();
        void scrape_video_kategori();
        void scrape_video_grup_dersleri();
        QList<std::pair<QString, QString>> lessons_to_scrape;
        QList<ScrapingLesson> lessons_scraped;
        std::pair<QString, QString> searching;
        bool lesson_names_scraped = false;
        template<typename Functor, typename OnError>
        void wait_for_element_to_appear(QString selector, Functor callback, OnError on_error, unsigned int timeout = 30);
        template<typename Functor>
        void wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout = 30);
};

#endif // SCRAPER_H
