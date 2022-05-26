#include "downloadlistdialog.h"
#include "ui_downloadlistdialog.h"
#include "ui_mainwindow.h"

#include <QProgressBar>
#include <QCloseEvent>
#include <QPushButton>
#include <mainwindow.h>

DownloadListDialog::DownloadListDialog(QWidget* parent) :
        QDialog(parent),
        ui(new Ui::DownloadListDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("İndirilenler"));
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->autoFillBackground();
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->horizontalHeader()->hide();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(this, &DownloadListDialog::cancel_download, this, &DownloadListDialog::download_cancelled);
}

DownloadListDialog::~DownloadListDialog()
{
    delete ui;
}

void DownloadListDialog::resizeEvent(QResizeEvent* event)
{
    for (int i = 0; i < ui->tableWidget->columnCount(); i++)
    {
        ui->tableWidget->setColumnWidth(i, ui->tableWidget->width() / ui->tableWidget->columnCount());
    }
    event->accept();
}

std::pair<QTableWidgetItem*, QProgressBar*> DownloadListDialog::findRowByName(const QString &name)
{
    auto item = ui->tableWidget->findItems(name, Qt::MatchFlag::MatchExactly);
    if (item.size() == 1)
    {
        auto p = ui->tableWidget->cellWidget(item[0]->row(), item[0]->column() + 1);
        QPointer<QProgressBar> pBar = qobject_cast<QProgressBar*>(p);
        return std::pair(item[0], pBar);
    } else return std::pair(nullptr, nullptr);
}

/**
 * @brief Append the given video_list to dialog's video list and return the added ones
 * @param video_list
 * @returns the difference
 */
QList<QPointer<Video>> DownloadListDialog::append_videos(const QList<QPointer<Video>> &video_list)
{
    QList<QPointer<Video>> difference;
    for (const auto &v: video_list)
    {
        auto it = std::find_if(this->videos.begin(), this->videos.end(),
                               [v](const QPointer<Video> &video) { return video->id == v->id; });
        if (it == this->videos.end())
        {
            // not found
            this->videos.push_back(v);
            difference.push_back(v);
        }
    }
    return difference;
}

void DownloadListDialog::download_started(const QList<QPointer<Video>> &video_list)
{
    QList<QPointer<Video>> v;
    if (!this->started)
    {
        this->started = true;
        for (const auto &video: video_list)
            this->videos.push_back(video);
        v = video_list;
    } else
    {
        v = this->append_videos(video_list);
    }
    for (const auto &video: v)
    {
        auto lastRow = ui->tableWidget->rowCount();
        auto item = new QTableWidgetItem(video->name);
        auto pBar = new QProgressBar(this);

        auto cancel_button = new QPushButton(QIcon(":/assets/cross.svg"), "");
        connect(cancel_button, &QPushButton::clicked, this, [this, video]() { emit this->cancel_download(video); });
        auto parent_widget = new QWidget(this);
        auto layout = new QHBoxLayout(parent_widget);
        layout->addWidget(cancel_button);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        parent_widget->setLayout(layout);

        pBar->setValue(0);
        ui->tableWidget->insertRow(lastRow);
        ui->tableWidget->setItem(lastRow, 0, item);
        ui->tableWidget->setCellWidget(lastRow, 1, pBar);
        ui->tableWidget->setCellWidget(lastRow, 2, parent_widget);
    }
    this->show();
}

void DownloadListDialog::download_progress(const QPointer<DownloadInfo> &info)
{
    auto p = this->findRowByName(info->video->name);
    if (p.first == nullptr || p.second == nullptr) return;
    p.second->setValue((int) info->percentage());
}

void DownloadListDialog::download_finished(const QPointer<DownloadInfo> &info)
{
    auto p = this->findRowByName(info->video->name);
    if (p.first == nullptr || p.second == nullptr) return;
    p.second->setValue(100);
}

void DownloadListDialog::closeEvent(QCloseEvent* event)
{
    this->hide();
    event->accept();
}

void DownloadListDialog::download_cancelled(const QPointer<Video> &video)
{
    auto p = this->findRowByName(video->name);
    if (p.first == nullptr || p.second == nullptr) return;
    ui->tableWidget->removeRow(p.first->row());
    this->videos.removeOne(video);
}
