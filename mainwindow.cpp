#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QWebEngineView>
#include <QFileDialog>
#include <QStandardPaths>
#include <iostream>
#include <QComboBox>
#include <scraper.h>

using namespace std::string_literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->loadLessonsFromFile();
    // set default location to downloads folder
    this->downloader.set_download_folder(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    this->download_list_dialog = new DownloadListDialog(this);
//    this->downloader.moveToThread(&this->worker_thread);
    connect(&this->downloader, &Downloader::downloadProgress, this->download_list_dialog, &DownloadListDialog::download_progress);
    connect(&this->downloader, &Downloader::downloadFinished, this->download_list_dialog, &DownloadListDialog::download_finished);
    connect(this, &MainWindow::start_download, this->download_list_dialog, &DownloadListDialog::download_started);
    connect(this, &MainWindow::start_download, &this->downloader, &Downloader::add_download);
//    connect(&this->worker_thread, &QThread::finished, &this->downloader, &QObject::deleteLater);
//    this->worker_thread.start();

    QStringList lesson_names;
    for (const auto& lesson: this->lessons) {
        auto f = lesson_names.filter(lesson->name);
        if (f.empty()) lesson_names.append(lesson->name);
    }

    for (const auto &name : lesson_names) {
        auto v = Lesson::filter_by_name(this->lessons, name);
        if (!v.empty()) {
            if (v.size() > 1) {
                // multiple teachers
                QVBoxLayout* layout = new QVBoxLayout;
                QComboBox* comboBox = new QComboBox;
                for (auto&& lesson: v) {
                    comboBox->addItem(lesson->teacher);
                }
                connect(comboBox, &QComboBox::currentTextChanged, this, &MainWindow::combobox_changed);
                QListWidget* l = this->buildListWidgetForLesson(v[0]);
                layout->addWidget(comboBox);
                layout->addWidget(l);
                QWidget* w = new QWidget;
                w->setLayout(layout);
                ui->tabWidget->addTab(w, name);
            } else {
                // single teacher
                auto lesson = v[0];
                auto l = this->buildListWidgetForLesson(lesson);
                ui->tabWidget->addTab(l, lesson->name);
            }
        }
    }

    auto s = new Scraper(this);
    s->show();
}

MainWindow::~MainWindow()
{
    delete ui;
    this->worker_thread.quit();
    this->worker_thread.wait();
}

void MainWindow::loadLessonsFromFile()
{
    QByteArray json_byte;
    QFile inputFile(":/lessons.json");

    if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        json_byte = inputFile.readAll();
        QJsonDocument doc(QJsonDocument::fromJson(json_byte));
        if (doc.isEmpty()) {
            QMessageBox::critical(this, tr("Empty json"), "Lessons.json is empty");
        }
        else {
            if (doc.isArray()) {
                QJsonArray arr = doc.array();
                for (auto item : arr) {
                    QJsonObject element = item.toObject();
                    QString id = element["id"].toString();
                    QString teacher = element["teacher"].toString();
                    QString name = element["name"].toString();
                    // TODO Add videos to lesson->videos
                    QPointer<Lesson> l = new Lesson(id, name, teacher);

                    QJsonArray videos_obj = element["videos"].toArray();
                    for (auto v_item : videos_obj) {
                        QJsonObject v_elem = v_item.toObject();
                        QString v_id = v_elem["id"].toString();
                        QString v_name = v_elem["name"].toString();
                        QString v_url = v_elem["url"].toString();
                        QPointer<Video> v = new Video(v_id, v_name, l->teacher, v_url);
                        v->lesson_name = l->name;
                        l->videos.push_back(v);
                    }
                    this->lessons.push_back(l);
                }
            }
        }
    } else {
        QMessageBox::critical(this, tr("Error"), inputFile.errorString());
    }
    inputFile.close();
}

QListWidget* MainWindow::buildListWidgetForLesson(QPointer<Lesson> lesson, QString objectName)
{
    QListWidget* l = new QListWidget;
    for (const auto& video : lesson->videos) {
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(video->name);
        item->setCheckState(Qt::CheckState::Unchecked);
        l->addItem(item);
    }
    if (objectName == nullptr) {
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


void MainWindow::on_actionUpdate_List_triggered()
{
    // TODO Ders Listesini Güncelle
}

void MainWindow::list_item_state_changed(QListWidgetItem* item)
{
    if (item->checkState() == Qt::CheckState::Checked) {
        bool found = false;
        for (const auto& l: this->lessons) {
            for (const auto& v: l->videos) {
                if (v->name == item->text()) {
                    this->videos_to_download.push_back(v);
                    found = true;
//                    qDebug() << "Added " << item->text() << " to download list";
                    break;
                }
            }
            if (found) break;
        }
    }
    else if (item->checkState() == Qt::CheckState::Unchecked) {
        int rv = this->videos_to_download.removeIf([item] (QPointer<Video> v) { return v->name == item->text(); });
//        if (rv>0)
//            qDebug() << "Removed " << item->text() << " from download list";
    }
}


void MainWindow::on_download_button_clicked()
{
    if (!videos_to_download.empty()) {
        emit start_download(videos_to_download);
    }
}

void MainWindow::combobox_changed(QString text)
{
    int i = ui->tabWidget->currentIndex();
    QString t = ui->tabWidget->tabText(i);
    auto l = Lesson::filter_by_name(this->lessons, t);
    auto lessons = Lesson::filter_by_teacher(l, text);

    if (lessons.size() != 1) {
        return;
    }
    auto lesson = lessons[0];
    QListWidget* listW = ui->tabWidget->findChild<QListWidget*>(lesson->name + "list");
    if (listW == nullptr) {
        return;
    }
    listW->clear();
    for (const auto& video: lesson->videos) {
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(video->name);
        item->setCheckState(Qt::CheckState::Unchecked);
        listW->addItem(item);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
}


void MainWindow::on_action_show_download_list_dialog_triggered()
{
    this->download_list_dialog->show();
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

