#ifndef LIVERECORDINGSCRAPER_H
#define LIVERECORDINGSCRAPER_H

#include <QMainWindow>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QPointer>
#include <QList>
#include <QString>
#include <QCloseEvent>
#include <QList>
#include <lesson.h>
#include <video.h>
#include <teacherlesson.h>


namespace Ui {
    class LiveRecordingScraper;
}

class LiveRecordingScraper : public QMainWindow
{
        Q_OBJECT

    public:
        explicit LiveRecordingScraper(QWidget *parent = nullptr);
        ~LiveRecordingScraper() override;
        [[nodiscard]] bool is_scraping() const;
        void scrape();

    private slots:
        void loading_finished();
        void start_new_lesson();
        void _on_get_next_video_info();
        void _on_acquired_new_video(QString name, QString src);
        void _on_finished_lesson(); // return to lesson list page
        void _on_finished();

    signals:
        void started();
        void new_video_scraped(QPointer<Video> video);
        void acquired_lesson_names(); // start new lesson
        void get_next_video_info();
        void acquired_new_video(QString name, QString src);
        void finished_lesson();
        void finished();

    private:
        Ui::LiveRecordingScraper *ui;
        QWebEngineProfile* profile = nullptr;
        QWebEnginePage* page = nullptr;
        QList<std::pair<TeacherLesson, QPointer<Lesson>>> lesson_list;
        std::pair<TeacherLesson, QPointer<Lesson>> current_lesson;
        bool lesson_names_scraped();

        void scrape_video();

        static QString video_page_dersler_listesi_class_name() { return QStringLiteral("DerslerListesi"); }
        static QString video_page_url_path() { return QStringLiteral("CanliVideoDersleri"); }
        static QString video_page_url_path_2() { return QStringLiteral("CanliDersBolum"); }

        void scrape_video_names();

        static QString lesson_list_page_url_path() { return QStringLiteral("CanliVideoKategori"); }
        static QString lesson_list_page_buttons_class_name() { return QStringLiteral("PnlIzle"); }
        void scrape_lesson_list_page();

        static QString online_konu_anlatimlari_url_path() { return QStringLiteral("CanliVideoAnaKategoriAlti"); }
        static QString online_konu_anlatimlari_button_class_name() { return QStringLiteral("VdDrKaSub"); }
        static QString online_konu_anlatimlari_search_string_1() { return QStringLiteral("Konu Anlatımları"); }
        static QString online_konu_anlatimlari_search_string_2() { return QStringLiteral("DUS"); }
        void nav_online_konu_anlatimlari();

        static QString canli_ders_kategori_url_path() { return QStringLiteral("CanliDersKategori"); }
        static QString canli_ders_kategori_button_class_name() { return QStringLiteral("VdCanliDersler"); }
        static QString canli_ders_kategori_search_string() { return QStringLiteral("Canlı Ders Kayıtları"); }
        void nav_canli_ders_dategori();

        static QString anasayfa_canli_dersler_button_class() { return QStringLiteral("VdRnkCn"); }
        static QString anasayfa_url_path() { return QStringLiteral("Anasayfa"); }
        void nav_anasayfa();


        bool working = false;

        template<typename Functor, typename OnError>
        void wait_for_element_to_appear(const QString& selector, Functor callback, OnError on_error, unsigned int timeout = 30);
        template<typename Functor>
        void wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout = 30);


    protected:
        void closeEvent(QCloseEvent* event) override;
};

#endif // LIVERECORDINGSCRAPER_H
