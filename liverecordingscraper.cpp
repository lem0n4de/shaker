#include "liverecordingscraper.h"
#include "ui_liverecordingscraper.h"

#include <QTimer>
#include <QNetworkCookie>
#include <QWebEngineCookieStore>
#include <QWebEngineSettings>
#include <QJsonArray>
#include <QJsonObject>
#include <jsondatabase.h>

LiveRecordingScraper::LiveRecordingScraper(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LiveRecordingScraper)
{
    ui->setupUi(this);
    this->setWindowTitle("Canlı Ders Scraper");

    connect(this, &LiveRecordingScraper::acquired_lesson_names, this, &LiveRecordingScraper::start_new_lesson);
    connect(this, &LiveRecordingScraper::get_next_video_info, this, &LiveRecordingScraper::_on_get_next_video_info);
    connect(this, &LiveRecordingScraper::acquired_new_video, this, &LiveRecordingScraper::_on_acquired_new_video);
    connect(this, &LiveRecordingScraper::finished_lesson, this, &LiveRecordingScraper::_on_finished_lesson);
    connect(this, &LiveRecordingScraper::finished, this, &LiveRecordingScraper::_on_finished);
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
    this->profile->setHttpUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 11_3) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1 Safari/605.1.15");

    auto cookie = QNetworkCookie("ASP.NET_SessionId", "f42gxbwtea1nzcy3250pr32w");
    cookie.setDomain("tusworld.com.tr");
    this->profile->cookieStore()->setCookie(cookie);
//    // END for now
    this->page = new QWebEnginePage(profile, this);
    connect(page, &QWebEnginePage::loadFinished, this, &LiveRecordingScraper::loading_finished);
    ui->webEngineView->setPage(page);

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
    } else if (url.path().contains(this->LESSON_LIST_PAGE_URL_PATH)) {
        if (this->lesson_names_scraped())
            this->start_new_lesson();
        else
            this->scrape_lesson_list_page();
    } else if (url.path().contains(this->VIDEO_PAGE_URL_PATH) || url.path().contains(this->VIDEO_PAGE_URL_PATH_2)) {
        if (this->current_lesson.first.is_started() &&
            this->current_lesson.first.video_infos.empty() &&
            !this->current_lesson.first.is_ended())
            this->scrape_video_names();
        else this->scrape_video();
    }
}

void LiveRecordingScraper::_on_acquired_new_video(QString name, QString src)
{
    QPointer<Video> v = new Video(name, "", src);
    emit this->new_video_scraped(v);
    this->current_lesson.second->videos.push_back(v);
    qDebug() << this->current_lesson.first.video_infos.size() << " videos left";
    emit this->get_next_video_info();
}

void LiveRecordingScraper::_on_finished_lesson()
{
    this->page->load(QUrl("https://www.tusworld.com.tr/" + this->LESSON_LIST_PAGE_URL_PATH));
}

void LiveRecordingScraper::_on_finished()
{
    QList<QPointer<Lesson>> list;
    for (const auto& p : this->lesson_list) {
        list.append(p.second);
    }
    auto l = JsonDatabase::retrieve_lessons();
    l.append(list);
    JsonDatabase::save_lessons(l);
    this->working = false;
}

bool LiveRecordingScraper::lesson_names_scraped()
{
    return !this->lesson_list.empty();
}

void LiveRecordingScraper::start_new_lesson()
{
    auto it = std::find_if(this->lesson_list.begin(),
                           this->lesson_list.end(),
                           [this] (std::pair<TeacherLesson, QPointer<Lesson>> pair) {
              return pair.first.video_infos.empty() && pair.second->videos.empty();
});
    if (it == this->lesson_list.end()) {
        emit this->finished();
        return;
    }
    this->current_lesson = *it;
    this->current_lesson.first.start();
    this->page->runJavaScript("document.getElementById('" + this->current_lesson.first.html_id + "').click();");
}

void LiveRecordingScraper::_on_get_next_video_info()
{
    if (this->current_lesson.first.is_started() && this->current_lesson.first.video_infos.empty()) {
        this->current_lesson.first.end();
        emit this->finished_lesson();
        qDebug() << "_on_get_next_video_info 1";
        return;
    }
    auto info = this->current_lesson.first.pop_video_info();
    if (this->current_lesson.first.video_infos.empty()) this->current_lesson.first.end();
    this->page->runJavaScript("document.getElementById('" + info.id + "').click();");
}

void LiveRecordingScraper::scrape_video()
{
    auto js = QString("(function () {"
                      "     let video_name = document.getElementsByClassName('VidAdi')[0].textContent.trim();"
                      "     let video_src = document.getElementsByTagName('video')[0].src;"
                      "     return { name: video_name, src: video_src };"
                      "})();");
    this->page->runJavaScript(js, [this] (const QVariant& out) {
        if (out.isValid()) {
            auto obj = out.toJsonObject();
            auto name = obj["name"].toString();
            auto src = obj["src"].toString();
            emit this->acquired_new_video(name, src);
        }
    });
}

void LiveRecordingScraper::scrape_video_names()
{
    qDebug() << "scrape_video_names";
    auto js = QString("(function() {"
                      "     let videos = [];"
                      "     let d_l = document.getElementsByClassName('" + this->VIDEO_PAGE_DERSLER_LISTESI_CLASS_NAME + "')[0].getElementsByTagName('a');"
                      "     for (let el of d_l) {"
                      "         videos.push({"
                      "             name: el.textContent.trim(),"
                      "             id: el.id"
                      "         });"
                      "     }"
                      "     return videos;"
                      "})();");
    if (this->current_lesson.first.video_infos.empty()) {
        this->page->runJavaScript(js, [this] (const QVariant& out) {
            if (out.isValid()) {
                auto arr = out.toJsonArray();
                for (const auto&& item: arr) {
                    auto obj = item.toObject();
                    auto name = obj["name"].toString();
                    auto id = obj["id"].toString();
                    TeacherLesson::video_info i { id, name };
                    this->current_lesson.first.video_infos.push_back(i);
                }
                this->page->runJavaScript("document.getElementById('" + this->current_lesson.first.video_infos[0].id + "').click();");
            }
        });
    }
}

void LiveRecordingScraper::scrape_lesson_list_page()
{
    auto js = QString("(function () {"
                      "     let lessons = [];"
                      "     for (let item of document.getElementsByClassName('"+ this->LESSON_LIST_PAGE_BTNS_CLASS_NAME + "')) {"
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
                this->lesson_list.push_back(std::pair(TeacherLesson(name, id), new Lesson(name, "")));
            }
            emit this->acquired_lesson_names();
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

void LiveRecordingScraper::enable_javascript(bool enable)
{
    this->page->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, enable);
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
