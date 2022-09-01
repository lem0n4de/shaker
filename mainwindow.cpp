#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QWebEngineView>
#include <QFileDialog>
#include <QStandardPaths>
#include <jsondatabase.h>
#include <iostream>
#include <QComboBox>
#include <QStatusBar>
#include <liverecordingscraper.h>
#include <scraper.h>

using namespace std::string_literals;

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Shaker");
    this->loadLessonsFromFile();
    this->download_list_dialog = new DownloadListDialog(this);

    connect(&this->downloader, &Downloader::downloadProgress, this->download_list_dialog,
            &DownloadListDialog::download_progress);
    connect(&this->downloader, &Downloader::downloadFinished, this->download_list_dialog,
            &DownloadListDialog::download_finished);
    connect(this, &MainWindow::start_download, this->download_list_dialog, &DownloadListDialog::download_started);
    connect(this, &MainWindow::start_download, &this->downloader, &Downloader::add_download);
    connect(this->download_list_dialog, &DownloadListDialog::cancel_download,
            &this->downloader, &Downloader::download_cancelled);
    connect(this->download_list_dialog, &DownloadListDialog::retry_download, this,
            [this](const QPointer<Video> &video) {
                this->downloader.add_download(QList{video});
            });

    QStringList lesson_names;
    for (const auto &lesson: this->lessons)
    {
        auto f = lesson_names.filter(lesson->name);
        if (f.empty()) lesson_names.append(lesson->name);
    }

    for (const auto &name: lesson_names)
    {
        auto v = Lesson::filter_by_name(this->lessons, name);
        if (!v.empty())
        {
            if (v.size() > 1)
            {
                // multiple teachers
                auto layout = new QVBoxLayout;
                auto comboBox = new QComboBox;
                for (auto &&lesson: v)
                {
                    comboBox->addItem(lesson->teacher);
                }
                connect(comboBox, &QComboBox::currentTextChanged, this, &MainWindow::combobox_changed);
                QListWidget* l = this->buildListWidgetForLesson(v[0]);
                layout->addWidget(comboBox);
                layout->addWidget(l);
                auto w = new QWidget;
                w->setLayout(layout);
                ui->tabWidget->addTab(w, name);
            } else
            {
                // single teacher
                auto lesson = v[0];
                auto l = this->buildListWidgetForLesson(lesson);
                ui->tabWidget->addTab(l, lesson->name);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    this->worker_thread.quit();
    this->worker_thread.wait();
}

void MainWindow::loadLessonsFromFile()
{
    try
    {
        this->lessons = JsonDatabase::retrieve_lessons();
    } catch (std::invalid_argument &e)
    {
        QMessageBox::critical(this, tr("Error"), tr(e.what()));
    }
}

QListWidget* MainWindow::buildListWidgetForLesson(const QPointer<Lesson> &lesson, QString objectName) const
{
    auto l = new QListWidget;
    for (const auto &video: lesson->videos)
    {
        auto item = new QListWidgetItem;
        item->setText(video->name);
        item->setCheckState(Qt::CheckState::Unchecked);
        l->addItem(item);
    }
    if (objectName == nullptr)
    {
        objectName = lesson->name + "list";
    }
    l->setObjectName(objectName);
    connect(l, &QListWidget::itemChanged, this, &MainWindow::list_item_state_changed);
    return l;
}


void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::list_item_state_changed(QListWidgetItem* item)
{
    if (item->checkState() == Qt::CheckState::Checked)
    {
        bool found = false;
        for (const auto &l: this->lessons)
        {
            for (const auto &v: l->videos)
            {
                if (v->name == item->text())
                {
                    this->videos_to_download.push_back(v);
                    found = true;
//                    qDebug() << "Added " << item->text() << " to download list";
                    break;
                }
            }
            if (found) break;
        }
    } else if (item->checkState() == Qt::CheckState::Unchecked)
    {
        std::remove_if(this->videos_to_download.begin(), this->videos_to_download.end(),
                       [item](const QPointer<Video> &v) { return v->name == item->text(); });
    }
}


void MainWindow::on_download_button_clicked()
{
    if (!videos_to_download.empty())
    {
        emit start_download(videos_to_download);
    }
}

void MainWindow::combobox_changed(const QString &text)
{
    int i = ui->tabWidget->currentIndex();
    QString t = ui->tabWidget->tabText(i);
    auto l = Lesson::filter_by_name(this->lessons, t);
    auto _lessons = Lesson::filter_by_teacher(l, text);

    if (_lessons.size() != 1)
    {
        return;
    }
    auto lesson = _lessons[0];
    auto listW = ui->tabWidget->findChild<QListWidget*>(lesson->name + "list");
    if (listW == nullptr)
    {
        return;
    }
    listW->clear();
    for (const auto &video: lesson->videos)
    {
        auto item = new QListWidgetItem;
        item->setText(video->name);
        item->setCheckState(Qt::CheckState::Unchecked);
        listW->addItem(item);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QMainWindow::closeEvent(event);
}


void MainWindow::on_action_show_download_list_dialog_triggered()
{
    this->download_list_dialog->show();
}

void MainWindow::on_new_video_scraped(const QPointer<Video> &video)
{
    this->ui->statusbar->showMessage(
            tr("Got url of [") + video->lesson_name + "(" + video->teacher + ") : " + video->name + "]");
}


void MainWindow::on_action_change_download_location_triggered()
{
    auto folder = QFileDialog::getExistingDirectory(this,
                                                    tr("İndirme lokasyonu seçin"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    this->downloader.set_download_folder(folder);
}


void MainWindow::on_action_recorded_lesson_refresh_triggered()
{
    if (this->live_recording_scraper && this->live_recording_scraper->is_scraping())
    {
        QMessageBox::critical(this, tr("Hata"), "Canlı dersler güncelleniyor, lütfen daha sonra tekrar deneyin.");
        return;
    }
    if (!this->scraper)
    {
        this->scraper = new Scraper(this);
        connect(this->scraper, &Scraper::new_video_scraped, this, &MainWindow::on_new_video_scraped);
    }
    this->scraper->scrape();
}


void MainWindow::on_action_live_lesson_refresh_triggered()
{
    if (this->scraper && this->scraper->is_scraping())
    {
        QMessageBox::critical(this, tr("Hata"), "Kayıtlı dersler güncelleniyor, lütfen daha sonra tekrar deneyin.");
        return;
    }
    if (!this->live_recording_scraper)
    {
        this->live_recording_scraper = new LiveRecordingScraper(this);
        connect(this->live_recording_scraper, &LiveRecordingScraper::new_video_scraped, this,
                &MainWindow::on_new_video_scraped);
    }
    this->live_recording_scraper->scrape();
}

