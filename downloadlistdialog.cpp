#include "downloadlistdialog.h"
#include "ui_downloadlistdialog.h"

#include <QProgressBar>

DownloadListDialog::DownloadListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadListDialog)
{
    ui->setupUi(this);
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

void DownloadListDialog::download_started(std::vector<Video*> videos)
{
    /* TODO
     * Connect DownloadListDialog with Downloader::downloadProgress
     * and update progress bars.
*/
    this->videos = videos;
    ui->tableWidget->setRowCount(videos.size());
    ui->tableWidget->setColumnCount(2);
    for (int i=0; i<this->videos.size(); i++) {
        auto video = videos[i];
        QTableWidgetItem* item = new QTableWidgetItem(video->name);
        QProgressBar* pBar = new QProgressBar;
        pBar->setValue(0);
        ui->tableWidget->setItem(i, 0, item);
        ui->tableWidget->setCellWidget(i, 1, pBar);
        widgets.push_back(std::pair(item, pBar));
    }
    ui->tableWidget->autoFillBackground();
    ui->tableWidget->setShowGrid(false);

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
