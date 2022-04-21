#ifndef DOWNLOADLISTDIALOG_H
#define DOWNLOADLISTDIALOG_H

#include <QDialog>
#include <video.h>
#include <downloaddata.h>
#include <QTableWidgetItem>
#include <QProgressBar>

namespace Ui {
    class DownloadListDialog;
}

class DownloadListDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DownloadListDialog(QWidget *parent = nullptr);
        ~DownloadListDialog();

    protected:
        void closeEvent(QCloseEvent* event) override;
    private:
        bool started;
        Ui::DownloadListDialog *ui;
        std::vector<Video*> videos;
        std::vector<std::pair<QTableWidgetItem*, QProgressBar*>> widgets;
        std::pair<QTableWidgetItem*, QProgressBar*> findRowByName(QString name);
        std::vector<Video*> append_videos(std::vector<Video*> videos);

    public slots:
        void download_started(std::vector<Video*> videos);
        void download_progress(DownloadData data);
        void download_finished(DownloadData data);
};

#endif // DOWNLOADLISTDIALOG_H
