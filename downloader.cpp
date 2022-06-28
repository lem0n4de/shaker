#include "downloader.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

Downloader::Downloader() = default;

Downloader::~Downloader()
{
    for (const auto &di: this->downloading)
    {
        if (di->file->isOpen()) di->file->close();
    }
}

void Downloader::set_download_folder(const QString &folder)
{
    QSettings settings;
    settings.setValue(Downloader::get_download_folder_setting_key(), folder);
    this->_download_folder = folder;

    qInfo() << "Set download folder to " << folder;
}

void Downloader::add_download(const QList<QPointer<Video>> &videos)
{
    for (const auto &video: videos)
    {
        auto it = std::find_if(this->downloading.begin(),
                               this->downloading.end(),
                               [video](const QPointer<DownloadInfo> &info) { return info->video == video; });
        if (it == this->downloading.end())
        {
            // not found, its not downloading
//            qDebug() << "Adding " << video->name << " to queue";
            this->queue.push_back(video);
        } else
        {
//            qDebug() << video->id << "\n" << video->name << "\n" << "ALREADY DOWNLOADING";
        }
    }
    if (this->queue.isEmpty())
    {
//        qDebug() << "Empty queue";
        return;
    } else download();
}

void Downloader::_on_download_progress(const QPointer<DownloadInfo> &info)
{
    emit downloadProgress(info);
}

void Downloader::_on_download_finished(const QPointer<DownloadInfo> &info)
{
    qDebug() << "Download finished: " << info->response->url();
    if (info->response->error() == QNetworkReply::OperationCanceledError)
    {
        info->error = "abort";
        info->file->remove();
    } else if (info->response->error() != QNetworkReply::NoError)
    {
        auto req = info->response->request();
        info->response->deleteLater();
        QPointer<QNetworkReply> reply = this->get_free_manager()->get(req);
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
            this->_on_download_progress(info);
        });
        connect(reply, &QNetworkReply::finished, this, [this, info]() { this->_on_download_finished(info); });
        connect(reply, &QNetworkReply::readyRead, this, [info]() { Downloader::_on_download_ready_read(info); });
    }
//    info->file->close();
    emit this->downloadFinished(info);
}

void Downloader::_on_download_ready_read(const QPointer<DownloadInfo> &info)
{
    info->file->write(info->response->readAll());
}

void Downloader::download()
{
    for (const auto &video: this->queue)
    {
        qDebug() << "Starting download of " << video->name;

        QDir default_downloads_dir(this->download_folder());
        QDir dir;

        if (!default_downloads_dir.mkpath(video->lesson_name))
        {
            qCritical()
                    << "Error creating directory: " << default_downloads_dir.absoluteFilePath(video->lesson_name)
                    << "\n"
                    << "Using default download location";
            dir = default_downloads_dir;
        } else dir = default_downloads_dir.absoluteFilePath(video->lesson_name);

        QString filename = dir.absoluteFilePath(video->name + ".mp4");
        QPointer<QFile> file = new QFile(filename);
        if (!file->open(QIODevice::WriteOnly))
        {
            qDebug() << "Error opening file: " << filename;
            return;
        }

        QNetworkRequest request(video->url);
        request.setRawHeader("referer", "https://tusworld.com.tr/VideoGrupDersleri");

        auto man = this->get_free_manager();
        this->increase_request_count(man);
        QPointer<QNetworkReply> reply = man->get(request);
        QPointer<DownloadInfo> download_info = new DownloadInfo(video, file, reply);
        /*
         * Somehow calculate speed.
         * Add timer to DownloadInfo, but it probably needs the usage of a single object,
         * Maybe turn all DownloadInfo to pointers again.
         */
        connect(reply, &QNetworkReply::downloadProgress, this,
                [this, download_info](qint64 bytesReceived, qint64 bytesTotal) {
                    download_info->total_length_in_bytes = bytesTotal / 1024;
                    download_info->completed_length_in_bytes = bytesReceived / 1024;
                    download_info->download_speed = 0;
                    this->_on_download_progress(download_info);
                });
        connect(reply, &QNetworkReply::finished, this, [this, download_info]() {
            this->_on_download_finished(download_info);
        });
        connect(reply, &QNetworkReply::readyRead, this, [this, download_info]() {
            this->_on_download_ready_read(download_info);
        });
        connect(man, &QNetworkAccessManager::finished, this, [this, man](QNetworkReply* reply) {
            this->decrease_request_count(man);
        });
        this->downloading.push_back(download_info);
        // speed = (bytes of data) / (time elapsed)
    }
    this->queue.clear();
}

QString Downloader::download_folder()
{
    if (!this->_download_folder.isEmpty())
    {
        return this->_download_folder;
    }
    QSettings settings;
    auto df = settings.value(Downloader::get_download_folder_setting_key(),
                             QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    this->_download_folder = df;
    return this->_download_folder;
}

void Downloader::download_cancelled(const QPointer<Video> &video)
{
    auto it = std::find_if(this->downloading.begin(),
                           this->downloading.end(),
                           [video](const QPointer<DownloadInfo> &info) { return info->video == video; });
    if (it == this->downloading.end())
    {
        qDebug() << "Download not found " << video->name;
        return;
    }
    auto di = *it;
    di->response->abort();
    this->downloading.removeOne(di);
    di->deleteLater();
    qDebug() << "Download cancelled " << di->video->name;
}

QPointer<QNetworkAccessManager> Downloader::get_free_manager()
{
    auto it = std::find_if(this->managers_and_counts.begin(), this->managers_and_counts.end(),
                           [](const std::pair<QPointer<QNetworkAccessManager>, int> &pair) {
                               if (pair.second <= 5) return true;
                               return false;
                           });

    if (it != this->managers_and_counts.end())
    {
        // found
        return (*it).first;
    }
    // not found
    qDebug() << "returning new manager";
    QPointer<QNetworkAccessManager> man = new QNetworkAccessManager;
    this->managers_and_counts.push_back(std::pair(man, 0));
    return man;
}

void Downloader::decrease_request_count(const QPointer<QNetworkAccessManager> &man)
{
    auto it = std::find_if(this->managers_and_counts.begin(), this->managers_and_counts.end(),
                           [man](const std::pair<QPointer<QNetworkAccessManager>, int> &pair) {
                               if (pair.first == man) return true;
                               return false;
                           });

    if (it != this->managers_and_counts.end())
        (*it).second -= 1;
}

void Downloader::increase_request_count(const QPointer<QNetworkAccessManager> &man)
{
    auto it = std::find_if(this->managers_and_counts.begin(), this->managers_and_counts.end(),
                           [man](const std::pair<QPointer<QNetworkAccessManager>, int> &pair) {
                               if (pair.first == man) return true;
                               return false;
                           });

    if (it != this->managers_and_counts.end())
        (*it).second += 1;
}


