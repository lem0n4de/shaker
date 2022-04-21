#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QThread>
#include <lesson.h>
#include <downloader.h>
#include <downloadlistdialog.h>
#include <aria2/aria2.h>

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

        void on_actionUpdate_List_triggered();
        void on_list_item_state_changed(QListWidgetItem* item);

        void on_download_button_clicked();
        void on_combobox_changed(QString text);
    signals:
        void start_download(std::vector<Video*> videos);

    private:
        Ui::MainWindow *ui;
        std::vector<Lesson*> lessons;
        std::vector<Video*> videos_to_download;
        aria2::Session* aria2_session;
        QThread worker_thread;
        Downloader downloader;
        void closeEvent(QCloseEvent *event) override;
        void loadLessonsFromFile();
        QListWidget* buildListWidgetForLesson(Lesson* lesson, QString objectName = nullptr);
        DownloadListDialog* download_list_dialog;
};
#endif // MAINWINDOW_H
