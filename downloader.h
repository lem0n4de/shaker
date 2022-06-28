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
        ~Downloader() override;
        void set_download_folder(const QString& folder);

    public slots:
        void add_download(const QList<QPointer<Video>>& videos);
        void download_cancelled(const QPointer<Video>& video);

    private slots:
        void _on_download_progress(const QPointer<DownloadInfo>& info);
        void _on_download_finished(const QPointer<DownloadInfo>& info);
        static void _on_download_ready_read(const QPointer<DownloadInfo>& info);

    signals:
        void downloadFinished(QPointer<DownloadInfo> data);
        void downloadProgress(QPointer<DownloadInfo> data);
        void downloadError(QPointer<DownloadInfo> data);
        void downloadStarted(QPointer<DownloadInfo> data);

    private:
        static QString get_download_folder_setting_key() { return "downloader/download_folder"; }
        QString _download_folder = "";
        QNetworkAccessManager manager;
        QList<std::pair<QPointer<QNetworkAccessManager>, int>> managers_and_counts;
        QList<QPointer<Video>> queue;
//        QList<QPointer<Video>> queue;
//        std::vector<DownloadInfo> replies;
        QList<QPointer<DownloadInfo>> downloading;
        void download();
        QString download_folder();

        QPointer<QNetworkAccessManager> get_free_manager();
        void increase_request_count(const QPointer<QNetworkAccessManager>& man);
        void decrease_request_count(const QPointer<QNetworkAccessManager>& man);
};

#endif // DOWNLOADER_H
