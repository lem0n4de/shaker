#include "downloadinfo.h"


DownloadInfo::DownloadInfo(const QPointer<Video> &video,
                           const QPointer<QFile> &file,
                           const QPointer<QNetworkReply> &response,
                           int64_t total_length,
                           int64_t completed_length,
                           int download_speed)
        : video(video), file(file), response(response), download_speed(download_speed),
          total_length_in_bytes(total_length), completed_length_in_bytes(completed_length)
{
}

DownloadInfo::DownloadInfo(const QPointer<Video> &video, const QPointer<QFile> &file,
                           const QPointer<QNetworkReply> &response)
        : video(video), file(file), response(response), download_speed(0), total_length_in_bytes(0),
          completed_length_in_bytes(0)
{
}

DownloadInfo::~DownloadInfo() = default;

int64_t DownloadInfo::total_length() const
{
    return this->total_length_in_bytes / 1024;
}

int64_t DownloadInfo::completed_length() const
{
    return this->completed_length_in_bytes / 1024;
}

double DownloadInfo::percentage() const
{
    return (double) ((double) this->completed_length() / (double) this->total_length()) * 100;
}
