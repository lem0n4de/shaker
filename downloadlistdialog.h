#ifndef DOWNLOADLISTDIALOG_H
#define DOWNLOADLISTDIALOG_H

#include <QDialog>
#include <video.h>
#include <downloadinfo.h>
#include <QTableWidgetItem>
#include <QProgressBar>
#include <QPointer>
#include <QList>

namespace Ui {
    class DownloadListDialog;
}

class DownloadListDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DownloadListDialog(QWidget *parent = nullptr);
        ~DownloadListDialog();
        void resizeEvent(QResizeEvent* event) override;
    signals:
        void cancel_download(const QPointer<Video> video);

    protected:
        void closeEvent(QCloseEvent* event) override;
    private:
        bool started;
        Ui::DownloadListDialog* ui;
        QList<QPointer<Video>> videos;
        std::pair<QTableWidgetItem*, QProgressBar*> findRowByName(QString name);
        QList<QPointer<Video>> append_videos(QList<QPointer<Video>> videos);

    public slots:
        void download_started(QList<QPointer<Video>> videos);
        void download_progress(QPointer<DownloadInfo> data);
        void download_finished(QPointer<DownloadInfo> data);
        void download_cancelled(const QPointer<Video>& video);
};

#endif // DOWNLOADLISTDIALOG_H
