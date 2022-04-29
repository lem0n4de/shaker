#include "scraper.h"
#include "ui_scraper.h"

#include <QTimer>
#include <QWebEngineProfile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Scraper::Scraper(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Scraper)
{
    ui->setupUi(this);

    profile = new QWebEngineProfile(this);
    profile->setHttpUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 12_3) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.3 Safari/605.1.15");
    page = new QWebEnginePage(profile, this);
    connect(page, &QWebEnginePage::loadFinished, this, &Scraper::loading_finished);
    ui->webEngineView->setPage(page);
    ui->webEngineView->showMaximized();
    page->load(QUrl("https://www.tusworld.com.tr/UyeGirisi"));

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
    } else if (url.matches(QUrl("https://www.tusworld.com.tr/VideoGrupDersleri"), QUrl::None)) {
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

void Scraper::scrape_video_grup_dersleri()
{

}

/**
 * @brief Scraper::wait_for_element_to_appear
 * @param selector
 * @param callback
 * @param timeout in seconds
 */
template<typename Functor>
void Scraper::wait_for_element_to_appear(QString selector, Functor callback, unsigned int timeout)
{
    int count = 0;
    auto max_tries = timeout / 1;
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, count, max_tries, timer, selector, callback] () {
        if (count == max_tries) {
            timer->stop();
            return;
        }
        page->runJavaScript("document.querySelector('" + selector + "');", 0,
                            [this, callback, timer] (const QVariant& out) {
            if (out.isValid()) {
                timer->stop();
                callback(out);
            }
        });
    });
    timer->start(1000);
}
