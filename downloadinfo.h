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
        DownloadInfo(QPointer<Video> video, QPointer<QFile> file, QPointer<QNetworkReply> response, int64_t total_length, int64_t completed_length, int download_speed);
        DownloadInfo(QPointer<Video> video, QPointer<QFile> file, QPointer<QNetworkReply> response);
        ~DownloadInfo();
        QPointer<Video> video;
        QPointer<QFile> file;
        QPointer<QNetworkReply> response;
        int download_speed; // In KB/s
        int64_t total_length(); // In KB
        int64_t completed_length(); // In KB
        int64_t total_length_in_bytes;
        int64_t completed_length_in_bytes;
        QString error;
        double percentage();
};

#endif // DOWNLOADINFO_H
