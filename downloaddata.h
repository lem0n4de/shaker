#ifndef DOWNLOADDATA_H
#define DOWNLOADDATA_H

#include <string>
#include <video.h>
#include <aria2/aria2.h>

using namespace std;

class DownloadData
{
    public:
        DownloadData(aria2::A2Gid gid, Video* video, string path, int64_t total_length, int64_t completed_length, double percentage, int download_speed);
        Video* video;
        string file_path;
        int64_t total_length;
        int64_t completed_length;
        double percentage;
        int download_speed;
        aria2::A2Gid gid;
        std::string error_message;
};

#endif // DOWNLOADDATA_H
