#ifndef SCRAPER_H
#define SCRAPER_H

#include <QMainWindow>
#include <QUrl>
#include <QWebEngineProfile>
#include <QWebEnginePage>

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

    private:
        Ui::Scraper *ui;
        QWebEngineProfile* profile;
        QWebEnginePage* page;
        void scrape_anasayfa();
        void scrape_video_kategori();
        void scrape_video_grup_dersleri();
        template<typename Functor>
        void wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout = 30);
};

#endif // SCRAPER_H
