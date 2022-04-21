#include "downloadlistdialog.h"
#include "ui_downloadlistdialog.h"

#include <QProgressBar>
#include <QCloseEvent>

DownloadListDialog::DownloadListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadListDialog)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->autoFillBackground();
    ui->tableWidget->setShowGrid(false);
}

DownloadListDialog::~DownloadListDialog()
{
    delete ui;
}

std::pair<QTableWidgetItem*, QProgressBar*> DownloadListDialog::findRowByName(QString name)
{
    auto it = std::find_if(widgets.begin(), widgets.end(), [name](std::pair<QTableWidgetItem*, QProgressBar*> pair) {
        return pair.first->text() == name;
    });
    if (it != widgets.end()) {
        return *it;
    }
    return std::pair(nullptr, nullptr);
}

/**
 * @brief Append the given videos to dialog's video list and return the added ones
 * @param videos
 * @returns the difference
 */
std::vector<Video*> DownloadListDialog::append_videos(std::vector<Video*> videos)
{
    std::vector<Video*> difference;
    for (auto&& v : videos) {
        auto it = std::find_if(this->videos.begin(), this->videos.end(), [v](Video* video) { return video->id == v->id; });
        if (it == this->videos.end()) {
            // not found
            this->videos.push_back(v);
            difference.push_back(v);
        }
    }
    return difference;
}

void DownloadListDialog::download_started(std::vector<Video*> videos)
{
    // TODO Sync videos with the new ones
    // TODO Add cancel option
    std::vector<Video*> v;
    if (!this->started) {
        this->started = true;
        for (auto video: videos)
            this->videos.push_back(video);
        v = videos;
    } else {
        v = this->append_videos(videos);
    }
    for (auto video: v) {
        auto lastRow = ui->tableWidget->rowCount();
        QTableWidgetItem* item = new QTableWidgetItem(video->name);
        QProgressBar* pBar = new QProgressBar(this);
        pBar->setValue(0);
        ui->tableWidget->insertRow(lastRow);
        ui->tableWidget->setItem(lastRow, 0, item);
        ui->tableWidget->setCellWidget(lastRow, 1, pBar);
        widgets.push_back(std::pair(item, pBar));
    }
    this->show();
}

void DownloadListDialog::download_progress(DownloadData data)
{
    auto p = this->findRowByName(data.video->name);
    if (p.first == nullptr || p.second == nullptr) return;
    p.second->setValue(data.percentage);
}

void DownloadListDialog::download_finished(DownloadData data)
{
    auto p = this->findRowByName(data.video->name);
    if (p.first == nullptr || p.second == nullptr) return;
    p.second->setValue(100);
}

void DownloadListDialog::closeEvent(QCloseEvent* event) {
    this->hide();
    event->accept();
}
