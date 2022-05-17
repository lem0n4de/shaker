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

        inline static const QString CANLI_DERSLER_BUTON_CLASS = QStringLiteral("VdRnkCn");
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
