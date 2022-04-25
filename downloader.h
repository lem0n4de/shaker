#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>
#include <QFile>
#include <QList>
#include <QPointer>
#include <video.h>
#include <downloadinfo.h>

class Downloader : public QObject
{
        Q_OBJECT
    public:
        Downloader();
        Downloader(QString download_folder);
        ~Downloader();
        void set_download_folder(QString folder);

    public slots:
        void add_download(QList<QPointer<Video>> videos);

    private slots:
        void on_download_progress(QPointer<DownloadInfo> info);
        void on_download_finished(QPointer<DownloadInfo> info);
        void on_download_ready_read(QPointer<DownloadInfo> info);

    signals:
        void downloadFinished(QPointer<DownloadInfo> data);
        void downloadProgress(QPointer<DownloadInfo> data);
        void downloadError(QPointer<DownloadInfo> data);
        void downloadStarted(QPointer<DownloadInfo> data);

    private:
        QString download_folder;
        QNetworkAccessManager manager;
        QQueue<QPointer<Video>> queue;
//        QList<QPointer<Video>> queue;
//        std::vector<DownloadInfo> replies;
        QList<QPointer<DownloadInfo>> downloading;
        void download();
};

#endif // DOWNLOADER_H
