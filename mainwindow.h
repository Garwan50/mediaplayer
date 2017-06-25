#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QFileDialog>
#include <QFile>
#include <QTime>
#include <QListWidgetItem>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QShortcut>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void PlaylistAdd(const QMediaContent &content);
    void PlaylistRemove(const QMediaContent &content);
    ~MainWindow();

public slots:
    void SetTime();
    void ImportMusic();
    void SetPosition();
    void SetPositionEnd();
    void SetRepeat();
    void SetTimeLeft();
    void SortList(int number);
    void ChangeMusic(QListWidgetItem* item);
    void PreviousMusic();
    void NextMusic();
    void ChangeMedia(QMediaContent media);
    void ActionSliderPosition(int action);
    void ActionSliderVolume(int action);
    void SavePlaylist();
    void LoadPlaylist();
    void SetTimer();
    void EndTimer();
    void PlayOrPause();
    void Play();
    void RemoveFromList();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QMediaPlaylist *playlist;
    QStringList *musicPath;
    QListWidgetItem *selected;
    QTimer *timer;
    QShortcut *pause;
    QShortcut *del;
};

#endif // MAINWINDOW_H
