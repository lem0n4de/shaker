#include "downloader.h"
#include <QDebug>
#include <QFile>

Downloader::Downloader()
{
}

Downloader::~Downloader()
{
}

void Downloader::add_download(QList<QPointer<Video>> videos)
{
    for (const auto& video: videos) {
        auto it = std::find_if(this->downloading.begin(),
                               this->downloading.end(),
                               [video](QPointer<DownloadInfo> info) { return info->video == video; });
        if (it == this->downloading.end()) {
            // not found, its not downloading
//            qDebug() << "Adding " << video->name << " to queue";
            this->queue.enqueue(video);
        } else {
//            qDebug() << video->id << "\n" << video->name << "\n" << "ALREADY DOWNLOADING";
        }
    }
    if (this->queue.isEmpty()){
//        qDebug() << "Empty queue";
        return;
    } else download();
}

void Downloader::on_download_progress(QPointer<DownloadInfo> info)
{
    emit downloadProgress(info);
}

void Downloader::on_download_finished(QPointer<DownloadInfo> info)
{
    qDebug() << "Download finished: " << info->response->url();
    info->file->close();
    emit this->downloadFinished(info);
}

void Downloader::on_download_ready_read(QPointer<DownloadInfo> info)
{
    info->file->write(info->response->readAll());
}

void Downloader::download()
{
    for (const auto& item: this->queue) {
        QPointer<Video> video = this->queue.dequeue();
        QString filename = video->name + ".mp4";
        QPointer<QFile> file = new QFile(filename);
        if (!file->open(QIODevice::WriteOnly)) {
            qDebug() << "Error opening file: " << filename;
            return;
        }

        QNetworkRequest request(video->url);
        request.setRawHeader("referer", "https://tusworld.com.tr/VideoGrupDersleri");

        QPointer<QNetworkReply> reply = this->manager.get(request);
        QPointer<DownloadInfo> download_info = new DownloadInfo(video, file, reply);
        /*
         * Somehow calculate speed.
         * Add timer to DownloadInfo, but it probably needs the usage of a single object,
         * Maybe turn all DownloadInfo to pointers again.
         */
        connect(reply, &QNetworkReply::downloadProgress, this, [this, download_info](qint64 bytesReceived, qint64 bytesTotal) {
            download_info->total_length_in_bytes = bytesTotal / 1024;
            download_info->completed_length_in_bytes = bytesReceived / 1024;
            download_info->download_speed = 0;
            this->on_download_progress(download_info);
        });
        connect(&this->manager, &QNetworkAccessManager::finished, this, [this, download_info] () {this->on_download_finished(download_info); });
        connect(reply, &QNetworkReply::readyRead, this, [this, download_info] () { this->on_download_ready_read(download_info); });
        this->downloading.push_back(download_info);
        // speed = (bytes of data) / (time elapsed)
    }
}


