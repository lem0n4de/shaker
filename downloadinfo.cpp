#include "downloadinfo.h"


DownloadInfo::DownloadInfo(QPointer<Video> video,
                           QPointer<QFile> file,
                           QPointer<QNetworkReply> response,
                           int64_t total_length,
                           int64_t completed_length,
                           int download_speed)
{
    this->video = video;
    this->file = file;
    this->response = response;
    this->total_length = total_length;
    this->completed_length = completed_length;
    this->download_speed = download_speed;
}

DownloadInfo::DownloadInfo(QPointer<Video> video, QPointer<QFile> file, QPointer<QNetworkReply> response)
{
    this->video = video;
    this->file = file;
    this->response = response;
}

DownloadInfo::~DownloadInfo()
{

}

double DownloadInfo::percentage()
{
    return (double) ((double) this->completed_length / (double) this->total_length)*100;
}
