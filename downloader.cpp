#include "downloader.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

Downloader::Downloader()
{
}

Downloader::Downloader(QString download_folder)
{
    this->set_download_folder(download_folder);
}

Downloader::~Downloader()
{
    for (const auto& di: this->downloading) {
        if (di->file->isOpen()) di->file->close();
    }
}

void Downloader::set_download_folder(QString folder)
{
    QSettings settings;
    settings.setValue(Downloader::get_download_folder_setting_key(), folder);
    this->_download_folder = folder;

    qInfo() << "Set download folder to " << folder;
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
    if (info->response->error() != QNetworkReply::NoError) {
        auto req = info->response->request();
        info->response->deleteLater();
        QPointer<QNetworkReply> reply = this->manager.get(req);
        info->response = reply;
        /*
         * Somehow calculate speed.
         * Add timer to DownloadInfo, but it probably needs the usage of a single object,
         * Maybe turn all DownloadInfo to pointers again.
         */
        connect(reply, &QNetworkReply::downloadProgress, this, [this, info](qint64 bytesReceived, qint64 bytesTotal) {
            info->total_length_in_bytes = bytesTotal / 1024;
            info->completed_length_in_bytes = bytesReceived / 1024;
            info->download_speed = 0;
            this->on_download_progress(info);
        });
        connect(reply, &QNetworkReply::finished, this, [this, info] () {this->on_download_finished(info); });
        connect(reply, &QNetworkReply::readyRead, this, [this, info] () { this->on_download_ready_read(info); });
    }
//    info->file->close();
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

        QDir default_downloads_dir(this->download_folder());
        QDir dir;

        if (!default_downloads_dir.mkpath(video->lesson_name)) {
            qCritical()
                    << "Error creating directory: " << default_downloads_dir.absoluteFilePath(video->lesson_name)<< "\n"
                    << "Using default download location";
            dir = default_downloads_dir;
        } else dir = default_downloads_dir.absoluteFilePath(video->lesson_name);

        QString filename = dir.absoluteFilePath(video->name + ".mp4");
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
        connect(reply, &QNetworkReply::finished, this, [this, download_info] () {this->on_download_finished(download_info); });
        connect(reply, &QNetworkReply::readyRead, this, [this, download_info] () { this->on_download_ready_read(download_info); });
        this->downloading.push_back(download_info);
        // speed = (bytes of data) / (time elapsed)
    }
}

QString Downloader::download_folder()
{
    if (!this->_download_folder.isEmpty()) {
        return this->_download_folder;
    }
    QSettings settings;
    auto df = settings.value(Downloader::get_download_folder_setting_key(),
                             QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    this->_download_folder = df;
    return this->_download_folder;
}


