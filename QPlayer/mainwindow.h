#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QFileDialog>
#include <QWidget>
#include <QVideoWidget>
#include <QtWidgets>
#include <QMediaPlaylist>

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
    void on_action_Open_triggered();

    void on_action_Play_triggered();

    void setPosition(int position);

    void durationChanged(qint64 duration);

    void seek(int seconds);

    void positionChanged(qint64 progress);

    void on_action_Mute_triggered(bool checked);

    void on_Rate_Fast_triggered();

    void on_Rate_Slow_triggered();

    void on_setFullScreen_triggered(bool checked);

    void on_setFullScreen_triggered();


private:
    Ui::MainWindow *ui;
    QString qsDataPath;
    QMediaPlayer *player;
    QVideoWidget *videoWidget;
    QSlider *slider;
    qint64 duration;
    QLabel *labelDuration;
    void updateDurationInfo(qint64 currentInfo);
    QAbstractSlider *volumeSlider;
    QMediaPlaylist *playlist;


};

class VideoWidget : public QVideoWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget * parent = 0);

protected:
    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

};
#endif // MAINWINDOW_H
