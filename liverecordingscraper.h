#ifndef LIVERECORDINGSCRAPER_H
#define LIVERECORDINGSCRAPER_H

#include <QMainWindow>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QPointer>
#include <QList>
#include <QString>
#include <QCloseEvent>
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

    private:
        Ui::LiveRecordingScraper *ui;
        QWebEngineProfile* profile;
        QWebEnginePage* page;

        inline static const QString ANASAYFA_CANLI_DERSLER_BUTON_CLASS = QStringLiteral("VdRnkCn");
        inline static const QString ANASAYFA_URL_PATH = QStringLiteral("Anasayfa");
        inline static const QString CANLI_DERS_KATEGORI_URL_PATH = QStringLiteral("CanliDersKategori");
        inline static const QString CANLI_DERS_KATEGORI_BUTON_CLASS = QStringLiteral("VdCanliDersler");
        inline static const QString CANLI_DERS_KATEGORI_SEARCH_STRING = QStringLiteral("Canlı Ders Kayıtları");
        void nav_canli_ders_dategori();
        void nav_anasayfa();

        bool working = false;

        template<typename Functor, typename OnError>
        void wait_for_element_to_appear(QString selector, Functor callback, OnError on_error, unsigned int timeout = 30);
        template<typename Functor>
        void wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout = 30);


    protected:
        void closeEvent(QCloseEvent* event) override;
};

#endif // LIVERECORDINGSCRAPER_H
