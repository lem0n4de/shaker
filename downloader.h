#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <video.h>
#include <downloaddata.h>

class Downloader : public QObject
{
        Q_OBJECT
    public:
        Downloader();
        ~Downloader();
        static int aria2_downloadEventCallback(aria2::Session* session, aria2::DownloadEvent event,
                                               const aria2::A2Gid gid, void* userData);
        DownloadData* filter_download_data_by_gid(aria2::A2Gid gid);

    public slots:
        void download(std::vector<Video*> videos);

    signals:
        void downloadFinished(DownloadData data);
        void downloadProgress(DownloadData data);
        void downloadError(DownloadData data);
        void downloadStarted(DownloadData data);

    private:
        std::vector<DownloadData*> v_download_data;
};

#endif // DOWNLOADER_H
