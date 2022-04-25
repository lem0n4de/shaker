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
            qDebug() << "Adding " << video->name << " to queue";
            this->queue.enqueue(video);
        } else {
            qDebug() << video->id << "\n" << video->name << "\n" << "ALREADY DOWNLOADING";
        }
    }

    qDebug() << "CALLING DOWNLOAD";
    download();
}

void Downloader::on_download_progress(qint64 bytes_received, qint64 bytes_total)
{
    qDebug() << "Bytes received: " << bytes_received << "\n" << "Total bytes: " << bytes_total;
}

void Downloader::on_download_progress(QPointer<DownloadInfo> info)
{
//    qDebug() << "NAME: " << info.video->name
//             << "\nPercentage: " << info.percentage
//             << "\nTotal length: " << info.total_length
//             << "\nCompleted Length: " << info.completed_length;
}

void Downloader::on_download_finished(QPointer<DownloadInfo> info)
{
    qDebug() << "Download finished: " << info->response->url();
    info->file->close();
}

void Downloader::on_download_ready_read(QPointer<DownloadInfo> info)
{
    info->file->write(info->response->readAll());
}

void Downloader::download()
{
    if (this->queue.isEmpty()){
        qDebug() << "EMPTY QUEUE";
        return;
    }

    qDebug() << "1";
    for (const auto& item: this->queue) {
        QPointer<Video> video = this->queue.dequeue();
        QString filename = video->name.append(".mp4");
        qDebug() << "Starting download of: " << filename;
        QPointer<QFile> file = new QFile(filename);

        qDebug() << "2";
        if (!file->open(QIODevice::WriteOnly)) {
            qDebug() << "Error opening file: " << filename;
            return;
        }

        QNetworkRequest request(video->url);
        request.setRawHeader("referer", "https://tusworld.com.tr/VideoGrupDersleri");
        qDebug() << "3";

        QPointer<QNetworkReply> reply = this->manager.get(request);
        QPointer<DownloadInfo> download_info = new DownloadInfo(video, file, reply);
        qDebug() << "4";
        /*
         * Somehow calculate speed.
         * Add timer to DownloadInfo, but it probably needs the usage of a single object,
         * Maybe turn all DownloadInfo to pointers again.
         */
        connect(reply, &QNetworkReply::downloadProgress, this, [this, download_info](qint64 bytesReceived, qint64 bytesTotal) {
            download_info->total_length = bytesTotal / 1024;
            download_info->completed_length = bytesReceived / 1024;
            download_info->download_speed = 0;
            this->on_download_progress(download_info);
        }); // &Downloader::on_download_progress);
        connect(&this->manager, &QNetworkAccessManager::finished, this, [this, download_info] () {this->on_download_finished(download_info); });
        connect(reply, &QNetworkReply::readyRead, this, [this, download_info] () { this->on_download_ready_read(download_info); });

        qDebug() << "5";
        this->downloading.push_back(download_info);
        qDebug() << "6";
        // speed = (bytes of data) / (time elapsed)
    }
    qDebug() << "END OF DOWNLOAD";
}


