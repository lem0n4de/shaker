#include "downloaddata.h"

DownloadData::DownloadData(aria2::A2Gid gid, Video* video, string path, int64_t total_length, int64_t completed_length, double percentage, int download_speed)
{
    this->gid = gid;
    this->video = video;
    this->file_path = path;
    this->total_length = total_length;
    this->completed_length = completed_length;
    this->percentage = percentage;
    this->download_speed = download_speed;
}
