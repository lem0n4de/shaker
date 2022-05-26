#include "scraper.h"
#include "ui_scraper.h"

#include <QTimer>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QNetworkCookie>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCloseEvent>
#include <jsondatabase.h>
#include <video.h>

Scraper::Scraper(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Scraper)
{
    ui->setupUi(this);
    this->setWindowTitle("QT WebEngine");

    connect(this, &Scraper::start_scrape_of_next_lesson, this, &Scraper::_on_start_scrape_of_next_lesson);
    connect(this, &Scraper::hc_atf_found, this, &Scraper::_on_hc_atf_found);
    connect(this, &Scraper::hc_atf_not_found, this, &Scraper::_on_hc_atf_not_found);
    connect(this, &Scraper::start_video_scrape_of_hc_atf_lesson, this, &Scraper::_on_start_video_scrape_of_hc_atf_lesson);
    connect(this, &Scraper::start_video_scrape_of_non_hc_atf_lesson, this, &Scraper::_on_start_video_scrape_of_non_hc_atf_lesson);
}

/**
 * @brief Scraper::scrape
 * Starts scraping the lessons. Silently returns if scraping
 * has already started.
 */
void Scraper::scrape()
{
    if (this->is_scraping()) {
        return;
    }
    this->_working = true;
    profile = new QWebEngineProfile(this);
    profile->setHttpUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 12_3) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.3 Safari/605.1.15");
//    // for now
    auto cookie = QNetworkCookie("ASP.NET_SessionId", "f42gxbwtea1nzcy3250pr32w");
    cookie.setDomain("tusworld.com.tr");
    profile->cookieStore()->setCookie(cookie);
//    // END for now
    page = new QWebEnginePage(profile, this);
    connect(page, &QWebEnginePage::loadFinished, this, &Scraper::loading_finished);
    ui->webEngineView->setPage(page);
    ui->webEngineView->showMaximized();

//    page->load(QUrl("https://www.tusworld.com.tr/UyeGirisi"));
    //     for now
    page->load(QUrl("https://www.tusworld.com.tr/Anasayfa"));
    // END for now
}

Scraper::~Scraper()
{
    delete ui;
}

bool Scraper::is_scraping() const
{
    return this->_working;
}

void Scraper::loading_finished()
{
    auto url = this->page->url();
    qDebug() << "Url == " << url;
    if (this->ongoing_video_scraping) {
        (this->*this->ongoing_video_scraping_function)();
    } else if (url.matches(QUrl("https://www.tusworld.com.tr/Anasayfa"), QUrl::None)) {
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
                       "        name: '" + this->searching_lesson_id_and_title.second + "',"
                       "        html_id: '" + this->searching_lesson_id_and_title.first + "',"
                       "        teacher: hcAtf.textContent.trim(),"
                       "        teacher_html_id: hcAtf.id"
                       "    });"
                       "    for (let el of document.getElementsByClassName('HocaAlt')) {"
                       "        lessons.push({"
                       "            name: '" + this->searching_lesson_id_and_title.second + "',"
                       "            html_id: '" + this->searching_lesson_id_and_title.first + "',"
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
                this->teacher_lessons.push_back(TeacherLesson(name, html_id, teacher, teacher_html_id));
            }
            emit this->start_video_scrape_of_hc_atf_lesson();
        }
    });
}

void Scraper::_on_hc_atf_not_found()
{
    this->teacher_lessons.push_back(TeacherLesson(this->searching_lesson_id_and_title.second, this->searching_lesson_id_and_title.first));
    emit this->start_video_scrape_of_non_hc_atf_lesson();
}

void Scraper::_on_start_video_scrape_of_hc_atf_lesson()
{
    if (this->ongoing_video_scraping) {
        if (this->current_lesson.first.video_infos.empty()) {
            this->get_video_names_for_current_lesson();
            /* Because get_video_names_for_current_lesson() clicks the first video
             * We need to return here and wait for page to load
             */
            return;
        }
        this->scrape_video_of_hc_atf_lesson_and_click_next_lesson();
        return;
    }

    QList<TeacherLesson> _list = this->build_remaining_lesson_list();

    if (!_list.empty()) {
        this->current_lesson.first = _list[0];
        _list.pop_front();
        auto lesson = new Lesson(this->current_lesson.first.name, this->current_lesson.first.teacher);
        this->finished_lessons.push_back(lesson);
        this->current_lesson.second = lesson;

        this->ongoing_video_scraping = true;
        this->ongoing_video_scraping_function = &Scraper::_on_start_video_scrape_of_hc_atf_lesson;

        this->click_element_by_id(this->current_lesson.first.teacher_html_id);
    } else {
        emit this->start_scrape_of_next_lesson();
    }
}

void Scraper::_on_start_video_scrape_of_non_hc_atf_lesson()
{
    if (this->ongoing_video_scraping) {
        if (this->current_lesson.first.video_infos.empty()) {
            this->get_video_names_for_current_lesson();
            return;
        }
        this->scrape_video_of_non_hc_atf_lesson_and_click_next_lesson();
        return;
    }
    QList<TeacherLesson> _list = this->build_remaining_lesson_list();

    if (!_list.empty()) {
        this->current_lesson.first = _list[0];
        _list.pop_front();
        auto lesson = new Lesson(this->current_lesson.first.name, this->current_lesson.first.teacher);
        this->finished_lessons.push_back(lesson);
        this->current_lesson.second = lesson;

        this->ongoing_video_scraping = true;
        this->ongoing_video_scraping_function = &Scraper::_on_start_video_scrape_of_non_hc_atf_lesson;

        this->click_element_by_id(this->current_lesson.first.html_id);
    } else {
        emit this->start_scrape_of_next_lesson();
    }
}

void Scraper::_on_start_scrape_of_next_lesson()
{
    if (this->lessons_to_scrape.isEmpty()) {
        if (!this->finished_lessons.isEmpty()) {
            // SAVE to lessons.json
            JsonDatabase::save_lessons(this->finished_lessons);
            this->_working = false;
            return;
        } else return;
    }
    this->searching_lesson_id_and_title = this->lessons_to_scrape[0];
    this->lessons_to_scrape.pop_front();
//    qDebug() << "Starting scraping of " << this->searching.second;
    auto js2 = QString("document.getElementById('"+ this->searching_lesson_id_and_title.first + "').click()");
    this->page->runJavaScript(js2);
}

void Scraper::scrape_video_grup_dersleri()
{
    // start scraping
    if (!this->lesson_names_scraped) {
        this->get_lesson_names();
    } else if (!this->ongoing_video_scraping) {
        // ongoing search
        this->wait_for_element_to_appear(".HcAtf",[this] (const QVariant& out) {
            qDebug() << "hc_atf";
            emit this->hc_atf_found();
        }, [this] () {
            qDebug() << "non_hc_atf";
            emit this->hc_atf_not_found();
        }, 10);
    } else {
        emit this->start_scrape_of_next_lesson();
    }
}

void Scraper::get_lesson_names()
{
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

/**
 * @brief Scraper::get_video_names_for_current_lesson
 * @details This function also calls the first video in
 * video infos as a side effect.
 */
void Scraper::get_video_names_for_current_lesson()
{
    this->wait_for_element_to_appear(".DerslerListesi",
                                     [this] (const QVariant& out) {
        if (out.isValid()) {
            this->page->runJavaScript("(function() {"
                                      "     let videos = [];"
                                      "     let d_l = document.getElementsByClassName('DerslerListesi')[0].getElementsByTagName('a');"
                                      "     for (let el of d_l) {"
                                      "         videos.push({"
                                      "             name: el.textContent.trim(),"
                                      "             id: el.id"
                                      "         });"
                                      "     }"
                                      "     return videos;"
                                      "})();",
                                      [this] (const QVariant& out) {
                if (out.isValid()) {
                    auto arr = out.toJsonArray();
                    for (auto&& item : arr) {
                        auto obj = item.toObject();
                        auto name = obj["name"].toString();
                        auto id = obj["id"].toString();
                        TeacherLesson::video_info i;
                        i.id = id;
                        i.name = name;
                        this->current_lesson.first.video_infos.push_back(i);
                    }

                    this->click_element_by_id(this->current_lesson.first.video_infos[0].id);
                }
            });
        }
    });
}

void Scraper::scrape_video_of_hc_atf_lesson_and_click_next_lesson()
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
            QPointer<Video> video = new Video(name, this->current_lesson.first.teacher, src);
            video->lesson_name = this->current_lesson.first.name;
            this->current_lesson.second->videos.push_back(video);
            qDebug() << "Scraped: " << video;

            emit this->new_video_scraped(video);
            this->current_lesson.first.video_infos.pop_front();
            if (!this->current_lesson.first.video_infos.empty()) {
                this->click_element_by_id(this->current_lesson.first.video_infos[0].id);
                qInfo() << this->current_lesson.first.video_infos.size() + 1 << " videos left";
            } else {
                this->ongoing_video_scraping = false;
                this->ongoing_video_scraping_function = nullptr;
                emit this->start_video_scrape_of_hc_atf_lesson();
            }
        }
    });
}

void Scraper::scrape_video_of_non_hc_atf_lesson_and_click_next_lesson()
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
            QPointer<Video> video = new Video(name, this->current_lesson.first.teacher, src);
            video->lesson_name = this->current_lesson.first.name;
            this->current_lesson.second->videos.push_back(video);
            qDebug() << "Scraped: " << video;

            emit this->new_video_scraped(video);
            this->current_lesson.first.video_infos.pop_front();
            if (!this->current_lesson.first.video_infos.empty()) {
                this->click_element_by_id(this->current_lesson.first.video_infos[0].id);
                qInfo() << this->current_lesson.first.video_infos.size() + 1 << " videos left";
            } else {
                this->ongoing_video_scraping = false;
                this->ongoing_video_scraping_function = nullptr;
                emit this->start_scrape_of_next_lesson();
            }
        }
    });
}

void Scraper::click_element_by_id(const QString& id)
{
    this->page->runJavaScript("document.getElementById('" + id + "').click();");
}

QList<TeacherLesson> Scraper::build_remaining_lesson_list()
{
    QList<TeacherLesson> _list;
    for (auto&& l: this->teacher_lessons) {
        auto it = std::find_if(this->finished_lessons.begin(),
                               this->finished_lessons.end(),
                               [l] (const QPointer<Lesson>& pLesson) {
                  return pLesson->name == l.name && pLesson->teacher == l.teacher;
        });
        if (it != this->finished_lessons.end()) continue;
        _list.push_back(l);
    }

    qDebug() << _list;
    return _list;
}

void Scraper::closeEvent(QCloseEvent* event)
{
    this->_working = false;
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
void Scraper::wait_for_element_to_appear(const QString& selector, Functor callback, OnError on_error, unsigned int timeout)
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
                            [callback, timer] (const QVariant& out) mutable {
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
