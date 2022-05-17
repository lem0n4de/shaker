#include "liverecordingscraper.h"
#include "ui_liverecordingscraper.h"

#include <QTimer>
#include <QNetworkCookie>
#include <QWebEngineCookieStore>
#include <QJsonArray>

LiveRecordingScraper::LiveRecordingScraper(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LiveRecordingScraper)
{
    ui->setupUi(this);
    this->setWindowTitle("Canlı Ders Scraper");
}

LiveRecordingScraper::~LiveRecordingScraper()
{
    delete ui;
}

bool LiveRecordingScraper::is_scraping()
{
    return this->working;
}

void LiveRecordingScraper::scrape()
{
    if (this->is_scraping()) {
        return;
    }

    this->working = true;
    this->profile = new QWebEngineProfile(this);
    this->profile->setHttpUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 12_3) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.3 Safari/605.1.15");

    auto cookie = QNetworkCookie("ASP.NET_SessionId", "f42gxbwtea1nzcy3250pr32w");
    cookie.setDomain("tusworld.com.tr");
    this->profile->cookieStore()->setCookie(cookie);
//    // END for now
    this->page = new QWebEnginePage(profile, this);
    connect(page, &QWebEnginePage::loadFinished, this, &LiveRecordingScraper::loading_finished);
    ui->webEngineView->setPage(page);
    ui->webEngineView->showMaximized();

//    page->load(QUrl("https://www.tusworld.com.tr/UyeGirisi"));
    //     for now
    this->page->load(QUrl("https://www.tusworld.com.tr/Anasayfa"));
    // END for now
}

void LiveRecordingScraper::loading_finished()
{
    auto url = this->page->url();
    qDebug() << "URL ==" << url.toDisplayString();
    if (url.path().contains(this->ANASAYFA_URL_PATH)) {
        this->nav_anasayfa();
    } else if (url.path().contains(this->CANLI_DERS_KATEGORI_URL_PATH)) {
        this->nav_canli_ders_dategori();
    } else if (url.path().contains(this->ONLINE_KONU_ANLATIMLARI_URL_PATH)) {
        this->nav_online_konu_anlatimlari();
    } else if (url.path().contains(this->VIDEO_LIST_PAGE_URL_PATH)) {
        this->scrape_video_list_page();
    }
}

void LiveRecordingScraper::scrape_video_list_page()
{
    auto js = QString("(function () {"
                      "     let lessons = [];"
                      "     for (let item of document.getElementsByClassName('"+ this->VIDEO_LIST_PAGE_BTNS_CLASS_NAME + "')) {"
                      "         let c = item.textContent.trim();"
                      "         let id = item.id;"
                      "         lessons.push([id, c]);"
                      "     }"
                      "     return lessons;"
                      "})();");
    this->page->runJavaScript(js, [this] (const QVariant& out) {
        if (out.isValid()) {
            auto arr = out.toJsonArray();
            for (const auto&& item: arr) {
                auto tuple = item.toArray();
                auto id = tuple[0].toString();
                auto name = tuple[1].toString().replace("Ders İzle", "").simplified();
                this->lesson_list.push_back(std::pair(TeacherLesson(name, id), nullptr));
            }
        }
    });
}

void LiveRecordingScraper::nav_online_konu_anlatimlari()
{
    auto js = QString("(function () {"
                      "     let elements = document.getElementsByClassName('" + this->ONLINE_KONU_ANLATIMLARI_BTN_CLASS_NAME + "');"
                      "     for (let item of elements) {"
                      "         if (item.textContent.trim().includes('" + this->ONLINE_KONU_ANLATIMLARI_SEARCH_STRING_1 + "') "
                                    "&& !item.textContent.trim().includes('" + this->ONLINE_KONU_ANLATIMLARI_SEARCH_STRING_2 + "')) {"
                      "             item.getElementsByTagName('a')[0].click();"
                      "         }"
                      "     }"
                      "})();");
    this->page->runJavaScript(js);
    qDebug() << "ran nav_online_konu_anlatimlari();";
}

void LiveRecordingScraper::nav_canli_ders_dategori()
{
    auto js = QString("(function () {"
                      "     let els = document.getElementsByClassName('" + this->CANLI_DERS_KATEGORI_BUTON_CLASS + "');"
                      "     for (let item of els) {"
                      "         if (item.textContent.trim().includes('" + this->CANLI_DERS_KATEGORI_SEARCH_STRING + "')) {"
                      "             item.getElementsByTagName('a')[0].click();"
                      "             return;"
                      "         }"
                      "     }"
                      "})();");
    this->wait_for_element_to_appear("." + this->CANLI_DERS_KATEGORI_BUTON_CLASS, [this, js] (const QVariant& out) {
        this->page->runJavaScript(js);
    });
}

void LiveRecordingScraper::nav_anasayfa()
{
    auto js = QString("document.getElementsByClassName('" +  this->ANASAYFA_CANLI_DERSLER_BUTON_CLASS + "')[0].click();");
    this->page->runJavaScript(js);
}






void LiveRecordingScraper::closeEvent(QCloseEvent* event)
{
    this->working = false;
    this->page->deleteLater();
    event->accept();
    this->deleteLater();
    QMainWindow::closeEvent(event);
}










/**
 * @brief Scraper::wait_for_element_to_appear
 * @param selector
 * @param callback
 * @param timeout in seconds
 */
template<typename Functor, typename OnError>
void LiveRecordingScraper::wait_for_element_to_appear(QString selector, Functor callback, OnError on_error, unsigned int timeout)
{
    auto max_tries = timeout / 1;
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, count = 0, max_tries, timer, selector, callback, on_error] () mutable {
//        qDebug () << "count == " << count;
        if (count == max_tries) {
            timer->stop();
//            qDebug() << "Calling on error";
            on_error();
            return;
        } else count += 1;
        page->runJavaScript("document.querySelector('" + selector + "');", 0,
                            [this, callback, timer] (const QVariant& out) mutable {
            if (out.isValid()) {
                timer->stop();
                callback(out);
            }
        });
    });
    timer->start(1000);
}

template<typename Functor>
void LiveRecordingScraper::wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout)
{
    this->wait_for_element_to_appear(selector, callback, [] () {}, timeout);
}
