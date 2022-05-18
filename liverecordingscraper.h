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
        ~LiveRecordingScraper();
        bool is_scraping();
        void scrape();

    private slots:
        void loading_finished();
        void start_new_lesson();
        void _on_get_next_video_info();
        void _on_acquired_new_video(QString name, QString src);
        void _on_finished_lesson(); // return to lesson list page
        void _on_finished();

    signals:
        /*
         * scrape start
         * lesson names acquired -> start new lesson -> get_next_video_info ->
         * acquired_new_video -> get_next_video_info ->
         * lesson done
         * scrape done
         */
        void started();
        void new_video_scraped(QPointer<Video> video);
        void acquired_lesson_names(); // start new lesson
        void get_next_video_info();
        void acquired_new_video(QString name, QString src);
        void finished_lesson();
        void finished();

    private:
        Ui::LiveRecordingScraper *ui;
        QWebEngineProfile* profile;
        QWebEnginePage* page;
        QList<std::pair<TeacherLesson, QPointer<Lesson>>> lesson_list;
        std::pair<TeacherLesson, QPointer<Lesson>> current_lesson;
        bool lesson_names_scraped();

        void scrape_video();

        inline static const QString VIDEO_PAGE_DERSLER_LISTESI_CLASS_NAME = QStringLiteral("DerslerListesi");
        inline static const QString VIDEO_PAGE_URL_PATH = QStringLiteral("CanliVideoDersleri");
        inline static const QString VIDEO_PAGE_URL_PATH_2 = QStringLiteral("CanliDersBolum");
        void scrape_video_names();

        inline static const QString LESSON_LIST_PAGE_URL_PATH = QStringLiteral("CanliVideoKategori");
        inline static const QString LESSON_LIST_PAGE_BTNS_CLASS_NAME = QStringLiteral("PnlIzle");
        void scrape_lesson_list_page();

        inline static const QString ONLINE_KONU_ANLATIMLARI_URL_PATH = QStringLiteral("CanliVideoAnaKategoriAlti");
        inline static const QString ONLINE_KONU_ANLATIMLARI_BTN_CLASS_NAME = QStringLiteral("VdDrKaSub");
        inline static const QString ONLINE_KONU_ANLATIMLARI_SEARCH_STRING_1 = QStringLiteral("Konu Anlatımları");
        inline static const QString ONLINE_KONU_ANLATIMLARI_SEARCH_STRING_2 = QStringLiteral("DUS");
        void nav_online_konu_anlatimlari();

        inline static const QString CANLI_DERS_KATEGORI_URL_PATH = QStringLiteral("CanliDersKategori");
        inline static const QString CANLI_DERS_KATEGORI_BUTON_CLASS = QStringLiteral("VdCanliDersler");
        inline static const QString CANLI_DERS_KATEGORI_SEARCH_STRING = QStringLiteral("Canlı Ders Kayıtları");
        void nav_canli_ders_dategori();

        inline static const QString ANASAYFA_CANLI_DERSLER_BUTON_CLASS = QStringLiteral("VdRnkCn");
        inline static const QString ANASAYFA_URL_PATH = QStringLiteral("Anasayfa");
        void nav_anasayfa();


        bool working = false;

        template<typename Functor, typename OnError>
        void wait_for_element_to_appear(QString selector, Functor callback, OnError on_error, unsigned int timeout = 30);
        template<typename Functor>
        void wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout = 30);

        void enable_javascript(bool enable);


    protected:
        void closeEvent(QCloseEvent* event) override;
};

#endif // LIVERECORDINGSCRAPER_H
