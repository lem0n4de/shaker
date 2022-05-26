#ifndef DOWNLOADINFO_H
#define DOWNLOADINFO_H

#include <QNetworkReply>
#include <QFile>
#include <QPointer>
#include <video.h>


class DownloadInfo: public QObject
{
        Q_OBJECT
    public:
        DownloadInfo(const QPointer<Video>& video, const QPointer<QFile>& file, const QPointer<QNetworkReply>& response, int64_t total_length, int64_t completed_length, int download_speed);
        DownloadInfo(const QPointer<Video>& video, const QPointer<QFile>& file, const QPointer<QNetworkReply>& response);
        ~DownloadInfo() override;
        QPointer<Video> video;
        QPointer<QFile> file;
        QPointer<QNetworkReply> response;
        int download_speed; // In KB/s
        [[nodiscard]] int64_t total_length() const; // In KB
        [[nodiscard]] int64_t completed_length() const; // In KB
        int64_t total_length_in_bytes;
        int64_t completed_length_in_bytes;
        QString error;
        [[nodiscard]] double percentage() const;
};

#endif // DOWNLOADINFO_H
