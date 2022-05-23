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
        void download_cancelled(const QPointer<Video>& video);

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
        static QString get_download_folder_setting_key() { return "downloader/download_folder"; }
        QString _download_folder = "";
        QNetworkAccessManager manager;
        QQueue<QPointer<Video>> queue;
//        QList<QPointer<Video>> queue;
//        std::vector<DownloadInfo> replies;
        QList<QPointer<DownloadInfo>> downloading;
        void download();
        QString download_folder();
};

#endif // DOWNLOADER_H
