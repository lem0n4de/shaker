#include "downloader.h"
#include <aria2/aria2.h>
#include <QDebug>

Downloader::Downloader()
{
    aria2::libraryInit();
}

Downloader::~Downloader()
{
    aria2::libraryDeinit();
    for (auto d: this->v_download_data) {
        if (d != nullptr) {
            delete d;
            d = nullptr;
        }
    }
}

int Downloader::aria2_downloadEventCallback(aria2::Session* session, aria2::DownloadEvent event,
                                            const aria2::A2Gid gid, void* userData) {
    Downloader* downloader = (Downloader *) userData;
    DownloadData* data;
    switch(event) {
    case aria2::EVENT_ON_DOWNLOAD_START:
        data = downloader->filter_download_data_by_gid(gid);
        if (data != nullptr) {
            data->download_speed = 0;
            data->completed_length = 0;
            data->total_length = 0;
            data->percentage = 0;
            emit downloader->downloadStarted(*data);
        }
        break;
    case aria2::EVENT_ON_DOWNLOAD_PAUSE:
        break;
    case aria2::EVENT_ON_DOWNLOAD_STOP:
        break;
    case aria2::EVENT_ON_DOWNLOAD_ERROR:
        data = downloader->filter_download_data_by_gid(gid);
        if (data != nullptr) {
            data->error_message = "Error";
        }
        break;
    case aria2::EVENT_ON_BT_DOWNLOAD_COMPLETE:
        break;
    case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
        data = downloader->filter_download_data_by_gid(gid);
        if (data != nullptr) {
            data->completed_length = data->total_length;
            data->percentage = 100.0;
            emit downloader->downloadFinished(*data);
        }
        break;
    }

    return 0;
}

DownloadData* Downloader::filter_download_data_by_gid(aria2::A2Gid gid)
{
    auto it = std::find_if(this->v_download_data.begin(),
                           this->v_download_data.end(),
                           [gid](DownloadData * data) { return gid == data->gid; });
    if (it != this->v_download_data.end()) {
        return *it;
    } else {
        return nullptr;
    }
}

void Downloader::download(std::vector<Video*> videos)
{
    aria2::Session* session;
    aria2::SessionConfig config;
    config.userData = this;
    aria2::KeyVals options;
    qDebug() << "options created";
    options.push_back(std::pair<std::string, std::string>("split", "16"));
    options.push_back(std::pair<std::string, std::string>("max-connection-per-server", "16"));
    qDebug() << "downloadCallback added";
    config.downloadEventCallback = Downloader::aria2_downloadEventCallback;
    session = aria2::sessionNew(options, config);
    qDebug() << "Aria2 session created";

    int rv;
    for (auto video: videos) {
        std::vector<std::string> uris = {video->url.toStdString()};
        std::string out = ""s + video->name.toStdString() + ".mp4";

        aria2::KeyVals d_opts;
        d_opts.push_back(std::pair<std::string, std::string>("out", out));
        d_opts.push_back(std::pair<std::string, std::string>("header", "referer: https://tusworld.com.tr/VideoGrupDersleri"));
        auto gid = aria2::hexToGid(video->id.toStdString());
        rv = aria2::addUri(session, &gid, uris, d_opts);
        if (rv <0) {
            qCritical() << "Failed to add downloads.";
            return;
        }
        auto d = new DownloadData(gid, video, out, -1, -1, -1, -1);
        this->v_download_data.push_back(d);
    }

    qDebug() << "v_download_data is ready";


    for (;;) {
        rv = aria2::run(session, aria2::RUN_ONCE);
        if (rv != 1) {
            break;
        }
        std::vector<aria2::A2Gid> gids = aria2::getActiveDownload(session);
        for (const auto& gid: gids) {
            aria2::DownloadHandle* dh = aria2::getDownloadHandle(session, gid);
            if (dh) {
                auto it = std::find_if(this->v_download_data.begin(), this->v_download_data.end(), [gid](DownloadData* d) {
                    return d->gid == gid;
                });
                if (it != this->v_download_data.end()) {
                    // found
                    auto data = *it;
                    data->completed_length = dh->getCompletedLength();
                    data->total_length = dh->getTotalLength();
                    data->download_speed = dh->getDownloadSpeed();
                    data->percentage = (double)data->completed_length / (double) data->total_length * 100.0;
                    qDebug() << "emit downloadProgress(*data)";
                    emit downloadProgress(*data);
                }
            }
        }
    }
}
