#include "mainwindow.h"
#include "ui_mainwindow.h"

VideoWidget::VideoWidget(QWidget *parent)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    setPalette(p);
    setAttribute(Qt::WA_OpaquePaintEvent);
}
void VideoWidget::keyPressEvent(QKeyEvent *event)
{

    switch (event->key())
    {
    case Qt::Key_Escape: //нажат Escape;
        setFullScreen(false);
        break;
    case Qt::Key_Up: //нажата Up;
        break;
    //...
    default:
        QVideoWidget::keyPressEvent(event);
        break;
    }
}
void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    setFullScreen(!isFullScreen());
    event->accept();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    videoWidget = new VideoWidget(this);
    player = new QMediaPlayer(this);
    player->setVideoOutput(videoWidget);
    ui->verticalLayout_2->addWidget(videoWidget);
    //Перемотка
    slider = new QSlider(Qt::Horizontal, this);
    ui->toolBar_2->addWidget(slider);
    connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)));
    connect(player, SIGNAL(durationChanged(qint64)), this,
            SLOT(durationChanged(qint64)));
    slider->setRange(0, player->duration() / 1000);

    labelDuration = new QLabel(this);
    ui->toolBar_2->addWidget(labelDuration);
    connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));
    connect(player, SIGNAL(positionChanged(qint64)),
            SLOT(positionChanged(qint64)));

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    ui->toolBar->addWidget(volumeSlider);
    connect(volumeSlider, SIGNAL(sliderMoved(int)), player, SLOT(setVolume(int)));
    volumeSlider->setValue(100);

    player->setPlaybackRate(player->playbackRate());
    ui->statusbar->showMessage("Скорость воспроизведения: " +QString::number(player->playbackRate()), 3000);

    playlist = new QMediaPlaylist();
    player->setPlaylist(playlist);

    connect(videoWidget, SIGNAL(signal_keyPress(QKeyEvent*)), this,
            SLOT(keyPressEvent(QKeyEvent*)));


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_action_Open_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Files"),
                                                          "C:\\TEMP");
    foreach (QString const &argument, fileNames)
    {
        QFileInfo fileInfo(argument);
        if (fileInfo.exists()) {
            QUrl url = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
            if (fileInfo.suffix().toLower() == QLatin1String("m3u")) {playlist->load(url);}
            else playlist->addMedia(url);}
        else {
            QUrl url(argument);
            if (url.isValid()) {playlist->addMedia(url);}
        }
    }
    player->setMedia(playlist->media(0));
    player->play();
    ui->tableWidget->show();
    if (ui->tableWidget->columnCount() == 0){ui->tableWidget->insertColumn(0);}
    int size = fileNames.size();
    int currentRow = ui->tableWidget->rowCount();
    for (int i = 0; i < size; ++i)
    {
        ui->tableWidget->insertRow(currentRow);
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(QFileInfo(fileNames[i]).fileName());
        ui->tableWidget->setItem(currentRow, 0, item);
        ++currentRow;
    }
    ui->tableWidget->setColumnWidth(0, this->width());
    if (ui->tableWidget->currentRow() < 0){ui->tableWidget->setCurrentItem(ui->tableWidget->item(0, 0));}

    ui->action_Play->setEnabled(true);
}

void MainWindow::on_action_Play_triggered()
{
switch(player->state()) {
case QMediaPlayer::PlayingState:
    player->pause(); break;
default:
    player->play(); break;
}
}


void MainWindow::setPosition(int position)
{
player->setPosition(position/1000);
}

void MainWindow::durationChanged(qint64 duration)
{
this->duration = duration/1000;
slider->setMaximum(duration / 1000);
}

void MainWindow::seek(int seconds)
{
player->setPosition(seconds * 1000);
}
void MainWindow::positionChanged(qint64 progress)
{
if (!slider->isSliderDown()) {
    slider->setValue(progress / 1000);
}
updateDurationInfo(progress / 1000);
}
void MainWindow::updateDurationInfo(qint64 currentInfo)
{
QString tStr;
if (currentInfo || duration) {
    QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60,
                      currentInfo%60, (currentInfo*1000)%1000);
    QTime totalTime((duration/3600)%60, (duration/60)%60, duration%60,
                    (duration*1000)%1000);
    QString format = "mm:ss";
    if (duration > 3600)
        format = "hh:mm:ss";
    tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
}
labelDuration->setText(tStr);
}



void MainWindow::on_action_Mute_triggered(bool checked)
{
    player->setMuted(checked);
}


void MainWindow::on_Rate_Fast_triggered()
{
    player->setPlaybackRate(player->playbackRate() + 1);
    ui->statusbar->showMessage("Скорость воспроизведения: " +QString::number(player->playbackRate()), 3000);
}


void MainWindow::on_Rate_Slow_triggered()
{
    player->setPlaybackRate(player->playbackRate() - 0.5);
    ui->statusbar->showMessage("Скорость воспроизведения: " +QString::number(player->playbackRate()), 3000);
}


void MainWindow::on_setFullScreen_triggered(bool checked)
{
    videoWidget->setFullScreen(!videoWidget->isFullScreen());

}



