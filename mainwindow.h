#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QThread>
#include <QPointer>
#include <QList>
#include <lesson.h>
#include <downloader.h>
#include <downloadlistdialog.h>

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
        void list_item_state_changed(QListWidgetItem* item);
        void on_download_button_clicked();
        void combobox_changed(QString text);
        void on_action_show_download_list_dialog_triggered();

    signals:
        void start_download(QList<QPointer<Video>> videos);

    private:
        Ui::MainWindow *ui;
        QList<QPointer<Lesson>> lessons;
        QList<QPointer<Video>> videos_to_download;
        QThread worker_thread;
        Downloader downloader;
        void closeEvent(QCloseEvent *event) override;
        void loadLessonsFromFile();
        QListWidget* buildListWidgetForLesson(Lesson* lesson, QString objectName = nullptr);
        DownloadListDialog* download_list_dialog;
};
#endif // MAINWINDOW_H
