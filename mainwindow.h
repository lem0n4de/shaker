#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QThread>
#include <QPointer>
#include <QList>
#include <QStandardPaths>
#include <lesson.h>
#include <downloader.h>
#include <downloadlistdialog.h>
#include <scraper.h>
#include <liverecordingscraper.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private slots:
        void on_actionExit_triggered();
        void list_item_state_changed(QListWidgetItem* item);
        void on_download_button_clicked();
        void combobox_changed(QString text);
        void on_action_show_download_list_dialog_triggered();
        void on_new_video_scraped(QPointer<Video> video);

        void on_action_change_download_location_triggered();
        void on_action_recorded_lesson_refresh_triggered();

        void on_action_live_lesson_refresh_triggered();

    signals:
        void start_download(QList<QPointer<Video>> videos);

    private:
        Ui::MainWindow *ui;
        QPointer<Scraper> scraper;
        QPointer<LiveRecordingScraper> live_recording_scraper;
        QList<QPointer<Lesson>> lessons;
        QList<QPointer<Video>> videos_to_download;
        QThread worker_thread;
        Downloader downloader;
        void closeEvent(QCloseEvent *event) override;
        void loadLessonsFromFile();
        QListWidget* buildListWidgetForLesson(QPointer<Lesson> lesson, QString objectName = nullptr);
        DownloadListDialog* download_list_dialog;
        QWidget* statusbar_textbox;
};
#endif // MAINWINDOW_H
