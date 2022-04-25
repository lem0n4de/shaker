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
    this->total_length_in_bytes = total_length;
    this->completed_length_in_bytes = completed_length;
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

int64_t DownloadInfo::total_length()
{
    return this->total_length_in_bytes / 1024;
}

int64_t DownloadInfo::completed_length()
{
    return this->completed_length_in_bytes / 1024;
}

double DownloadInfo::percentage()
{
    return (double) ((double) this->completed_length() / (double) this->total_length())*100;
}
