#include "scraper.h"
#include "ui_scraper.h"

#include <QTimer>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QNetworkCookie>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Scraper::Scraper(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Scraper)
{
    ui->setupUi(this);

    connect(this, &Scraper::start_scrape_of_next_lesson, this, &Scraper::_on_start_scrape_of_next_lesson);
    connect(this, &Scraper::hc_atf_found, this, &Scraper::_on_hc_atf_found);
    connect(this, &Scraper::hc_atf_not_found, this, &Scraper::_on_hc_atf_not_found);
    connect(this, &Scraper::start_video_scrape_of_hc_atf_lesson, this, &Scraper::_on_start_video_scrape_of_hc_atf_lesson);
    connect(this, &Scraper::start_video_scrape_of_non_hc_atf_lesson, this, &Scraper::_on_start_video_scrape_of_non_hc_atf_lesson);

    profile = new QWebEngineProfile(this);
    profile->setHttpUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 12_3) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.3 Safari/605.1.15");
    // for now
    auto cookie = QNetworkCookie("ASP.NET_SessionId", "kpokrsk1sua1mnew3izhd2vb");
    cookie.setDomain("tusworld.com.tr");
    profile->cookieStore()->setCookie(cookie);
    // END for now
    page = new QWebEnginePage(profile, this);
    connect(page, &QWebEnginePage::loadFinished, this, &Scraper::loading_finished);
    ui->webEngineView->setPage(page);
    ui->webEngineView->showMaximized();
    // for now
//    page->load(QUrl("https://www.tusworld.com.tr/UyeGirisi"));
    page->load(QUrl("https://www.tusworld.com.tr/Anasayfa"));
    // END for now
}

Scraper::~Scraper()
{
    delete ui;
}

void Scraper::loading_finished()
{
    auto url = this->page->url();
    qDebug() << "Url == " << url;
    if (url.matches(QUrl("https://www.tusworld.com.tr/Anasayfa"), QUrl::None)) {
        this->scrape_anasayfa();
    } else if (url.matches(QUrl("https://www.tusworld.com.tr/VideoKategori"), QUrl::None)) {
        // okul öğrencilerine premium paket
        this->scrape_video_kategori();
    } else if (url.matches(QUrl("https://www.tusworld.com.tr/VideoGrupDersleri"), QUrl::None)
               || url.matches(QUrl("https://www.tusworld.com.tr/GrupVidDersBolumleri"), QUrl::None)) {
        // Ders Kategorileri
        this->scrape_video_grup_dersleri();
    }
}

void Scraper::scrape_anasayfa()
{
    this->wait_for_element_to_appear("#tclose", [this] (const QVariant& out) {
        this->page->runJavaScript("document.querySelector('#tclose').click()");
    });
    this->wait_for_element_to_appear(".VdRnk", [this] (const QVariant& out) {
        this->page->runJavaScript("document.querySelector('.VdRnk').click()");
    });
    this->wait_for_element_to_appear(".Tusblue", [this] (const QVariant& out) {
        this->page->runJavaScript("document.querySelector('.Tusblue').click()");
    });

    this->wait_for_element_to_appear(".VdDrKaSub", [this] (const QVariant& out) {
        // loop over all VdDrKaSub elements and found "evde" and "offline" in their texts
        // if found break loop then click it
        // all in javascript
        auto js = QStringLiteral("for (item of document.getElementsByClassName('VdDrKaSub')) {"
                                     "if (item.innerText.toLowerCase().includes('evde') &&"
                                         "item.innerText.toLowerCase().includes('offline')) {"
                                            "item.getElementsByTagName('a')[0].click();"
                                      "}"
                                 "}");
        this->page->runJavaScript(js);
    });
}

void Scraper::scrape_video_kategori()
{
    this->wait_for_element_to_appear("h1", [this] (const QVariant& out) {
        auto js = QStringLiteral("for (item of document.getElementsByTagName('h1')) {"
                                    "if (item.innerText.toLowerCase().includes('okul öğrencilerine premium paket')) {"
                                        "item.click();"
                                    "}"
                                 "}");
        this->page->runJavaScript(js);
    });
}

void Scraper::_on_hc_atf_found()
{
    auto js3 = QString("(function() {"
                       "    let lessons = [];"
                       "    let hcAtf = document.getElementsByClassName('HcAtf')[0];"
                       "    lessons.push({"
                       "        name: '" + this->searching.second + "',"
                       "        html_id: '" + this->searching.first + "',"
                       "        teacher: hcAtf.textContent.trim(),"
                       "        teacher_html_id: hcAtf.id"
                       "    });"
                       "    for (let el of document.getElementsByClassName('HocaAlt')) {"
                       "        lessons.push({"
                       "            name: '" + this->searching.second + "',"
                       "            html_id: '" + this->searching.first + "',"
                       "            teacher: el.textContent.trim(),"
                       "            teacher_html_id: el.id,"
                       "        });"
                       "    }"
                       "    return lessons;"
                       "})();");

    this->page->runJavaScript(js3, [this] (const QVariant& out) {
        if (out.isValid()) {
            auto arr = out.toJsonArray();
            for (auto&& l: arr) {
                auto obj = l.toObject();
                auto name = obj["name"].toString();
                auto teacher = obj["teacher"].toString();
                auto html_id = obj["html_id"].toString();
                auto teacher_html_id = obj["teacher_html_id"].toString();
                this->lessons_scraped.push_back(ScrapingLesson(name, html_id, teacher, teacher_html_id));
            }
            emit this->start_video_scrape_of_hc_atf_lesson();
        }
    });
}

void Scraper::_on_hc_atf_not_found()
{
    this->lessons_scraped.push_back(ScrapingLesson(this->searching.second, this->searching.first));
    emit this->start_video_scrape_of_non_hc_atf_lesson();
}

void Scraper::_on_start_video_scrape_of_hc_atf_lesson()
{
    qDebug() << "on_start_video_scrape_of_hc_atf_lesson() - " << this->searching.second;
    emit this->start_scrape_of_next_lesson();
}

void Scraper::_on_start_video_scrape_of_non_hc_atf_lesson()
{
    qDebug() << "on_start_video_scrape_of_non_hc_atf_lesson() - " << this->searching.second;
    emit this->start_scrape_of_next_lesson();
}

void Scraper::_on_start_scrape_of_next_lesson()
{
    if (this->lessons_to_scrape.isEmpty()) return;
    this->searching = this->lessons_to_scrape[0];
    this->lessons_to_scrape.pop_front();
//    qDebug() << "Starting scraping of " << this->searching.second;
    auto js2 = QString("document.getElementById('"+ this->searching.first + "').click()");
    this->page->runJavaScript(js2);
}

void Scraper::scrape_video_grup_dersleri()
{
    if (this->searching.second.isEmpty()) {
        // start scraping
        if (!this->lesson_names_scraped) {
            // only scrape lesson names if we did not already
            // otherwise multiple runJavascript's will shoot.
            auto js1 = QStringLiteral("(function () {"
                                      "     let lessons = [];"
                                      "     let dk = document.getElementsByClassName('DersKategorileri')[0];"
                                      "     for (let t of dk.getElementsByTagName('a')) {"
                                      "         lessons.push([t.id, t.textContent.trim()])"
                                      "     }"
                                      "     return lessons;"
                                      "})();");
            this->page->runJavaScript(js1, [this] (const QVariant& out) {
                if (out.isValid()) {
                    auto arr = out.toJsonArray();
                    for (auto&& item: arr) {
                        auto id = item.toArray()[0].toString();
                        auto text = item.toArray()[1].toString();
                        this->lessons_to_scrape.push_back(std::pair(id, text));
                    }
//                    qDebug() << this->lessons_to_scrape;
                    this->lesson_names_scraped = true;

                    emit this->start_scrape_of_next_lesson();
                }
            });
        }
    } else {
        // ongoing search
//        qDebug() << "ongoing search for " << this->searching.second;
        this->wait_for_element_to_appear(".HcAtf",[this] (const QVariant& out) {
//            qDebug() << ".HcAtf found for " << this->searching.second;
            emit this->hc_atf_found();
        }, [this] () {
//            qDebug() << ".HcAtf not found for " << this->searching.second;
            emit this->hc_atf_not_found();
        }, 10);
    }
}

/**
 * @brief Scraper::wait_for_element_to_appear
 * @param selector
 * @param callback
 * @param timeout in seconds
 */
template<typename Functor, typename OnError>
void Scraper::wait_for_element_to_appear(QString selector, Functor callback, OnError on_error, unsigned int timeout)
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
void Scraper::wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout)
{
    this->wait_for_element_to_appear(selector, callback, [] () {}, timeout);
}
