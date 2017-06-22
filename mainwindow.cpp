#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Lecteur média");
    player = new QMediaPlayer;
    playlist = new QMediaPlaylist;
    musicPath = new QStringList;
    selected = new QListWidgetItem;
    selected = NULL;
    timer = new QTimer(this);
    pause = new QShortcut(QKeySequence("SPACE"), this);

    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(SetTime()));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(SetTimeLeft()));
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(ChangeMedia(QMediaContent)));
    connect(ui->button_pause, SIGNAL(clicked(bool)), player, SLOT(pause()));
    connect(ui->button_play, SIGNAL(clicked(bool)), this, SLOT(Play()));
    connect(ui->slider_volume, SIGNAL(actionTriggered(int)), this, SLOT(ActionSliderVolume(int)));
    connect(ui->slider_volume, SIGNAL(sliderMoved(int)), player, SLOT(setVolume(int)));
    connect(ui->button_next, SIGNAL(clicked(bool)), this, SLOT(NextMusic()));
    connect(ui->button_previous, SIGNAL(clicked(bool)), this, SLOT(PreviousMusic()));
    connect(ui->button_import, SIGNAL(clicked(bool)), this, SLOT(ImportMusic()));
    connect(ui->slider_position, SIGNAL(sliderReleased()), this, SLOT(SetPosition()));
    connect(ui->slider_position, SIGNAL(actionTriggered(int)), this, SLOT(ActionSliderPosition(int)));
    connect(ui->radio_repeat, SIGNAL(toggled(bool)), this, SLOT(SetRepeat()));
    connect(ui->combo_sort, SIGNAL(activated(int)), this, SLOT(SortList(int)));
    connect(ui->list_song, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(ChangeMusic(QListWidgetItem*)));
    connect(ui->button_save, SIGNAL(clicked(bool)), this, SLOT(SavePlaylist()));
    connect(ui->button_load, SIGNAL(clicked(bool)), this, SLOT(LoadPlaylist()));
    connect(ui->button_timer, SIGNAL(clicked(bool)), this, SLOT(SetTimer()));
    connect(timer, SIGNAL(timeout()), this, SLOT(EndTimer()));
    connect(pause, SIGNAL(activated()), this, SLOT(PlayOrPause())); //

    timer->setSingleShot(true);

    player->setPlaylist(playlist);
    player->setVolume(50);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Play() {
    if(playlist->isEmpty()) return;
    else if(!playlist->isEmpty() && playlist->currentMedia().isNull()) return;
    else player->play();
}

void MainWindow::PlayOrPause() {
    if(player->state() == QMediaPlayer::PausedState) {
        this->Play();
    } else if (player->state() == QMediaPlayer::PlayingState) {
        player->pause();
    }
}

void MainWindow::SetTime() {
    if(player->state() == QMediaPlayer::PausedState) return;
    float durationMax = player->duration();
    float currentDuration = player->position();
    float value = 0;
    if(durationMax != 0) {
    value = (currentDuration / durationMax) * 100;
    }
    ui->slider_position->setValue(value);
}

void MainWindow::PlaylistAdd(const QMediaContent &content) {
    playlist->addMedia(content);
    QString label;
    label = content.canonicalUrl().toLocalFile();
    qDebug() << content.canonicalUrl();
    qDebug() << label;
    musicPath->append(label);
    label = label.section("/", -1);
    qDebug() << label;
    ui->list_song->addItem(label);
}

void MainWindow::ImportMusic() {

    QStringList fileName = QFileDialog::getOpenFileNames(this, "Importer vos musique(s)", QStandardPaths::writableLocation(QStandardPaths::MusicLocation), "Fichier musicaux (*.mp3 *.wav *.flac *.wma)");
    for(int i = 0; i < fileName.size(); i++) {
        PlaylistAdd(QUrl::fromLocalFile(fileName[i]));
    }
}

void MainWindow::SetPosition() {
    float duration = player->duration(); // ex 1000
    float position = ui->slider_position->value(); // ex 30 sur 100
    float value = (position/100) * duration;
    player->setPosition(value); // = 300
}

void MainWindow::ActionSliderPosition(int action) {
    player->pause();
    if(action == QAbstractSlider::SliderPageStepAdd || action == QAbstractSlider::SliderPageStepSub) {
        float duration = player->duration(); // ex 1000
        float position = ui->slider_position->value(); // ex 30
        float value = (position/100) * duration; // ex 30/100 * 1000
        player->setPosition(value); // = 300
    }
    player->play();
}
void MainWindow::ActionSliderVolume(int action) {
    if(action == QAbstractSlider::SliderPageStepAdd || action == QAbstractSlider::SliderPageStepSub) {
        player->setVolume(ui->slider_volume->value());
    }
}

void MainWindow::SetRepeat() {
    if(ui->radio_repeat->isChecked()) playlist->setPlaybackMode(QMediaPlaylist::Loop);
    else playlist->setPlaybackMode(QMediaPlaylist::Sequential);
}

void MainWindow::SetTimeLeft() {
    long int ms = player->duration()-player->position();
    QTime time = QTime(0, 0, 0);
    time = time.addMSecs(ms);
    ui->label_duration->setText(time.toString("mm:ss"));
}

void MainWindow::SortList(int number) {
    if(number == 1) { // sort random
        //on sauvegarde la musique en cours
        QMediaContent media = playlist->currentMedia();
        int position = (int)player->position();
        // generation d'une liste de nombre aléatoire
        srand(QDateTime::currentMSecsSinceEpoch());
        QList<int> randoms;
        for(int i = 0; i < musicPath->count(); i++){
            randoms.append(qrand());
        }
        // tri relatif au tableau randoms
        int index;
        QStringList newTab1;
        for(int j = 0; j < musicPath->count(); j++) {
            index = std::distance(randoms.begin(), std::min_element(randoms.begin(), randoms.end()));
            newTab1.append(musicPath->at(index));
            randoms[index] = RAND_MAX;
        }
        // Reinitialisation de la playlist
        selected->setBackground(QColor(0, 0, 0, 0));
        selected = NULL;
        playlist->clear();
        ui->list_song->clear();
        musicPath->clear();
        // generation de la nouvelle playlist
        bool found = false;
        for(int i = 0; i < newTab1.count(); i++) {
            PlaylistAdd(QUrl::fromLocalFile(newTab1[i]));
            if(playlist->media(i) == media && !found) {
                index = i;
                found = true;
            }
        }
        playlist->setCurrentIndex(index);
        if(position > 0) {
            selected = ui->list_song->item(index);
            selected->setBackground(QColor(14,193,251,100));
            player->setPosition(position);
            player->play();
        }
    }
}

void MainWindow::PreviousMusic() {
    if(playlist->media(playlist->currentIndex()-1).isNull()) return;
    int index = playlist->currentIndex()-1;

    selected->setBackground(QColor(0, 0, 0, 0));
    selected = ui->list_song->item(index);
    selected->setBackground(QColor(14,193,251,100));
    playlist->setCurrentIndex(index);
    player->play();
}

void MainWindow::NextMusic() {
    int index = playlist->currentIndex()+1;
    if(playlist->media(playlist->currentIndex()+1).isNull()) index = 0;

    if(selected != NULL) {
        selected->setBackground(QColor(0, 0, 0, 0));
    }
    selected = ui->list_song->item(index);
    selected->setBackground(QColor(14,193,251,100));
    playlist->setCurrentIndex(index);
    player->play();
}

void MainWindow::ChangeMusic(QListWidgetItem* item) {
    if(selected != NULL) {
        selected->setBackground(QColor(0, 0, 0, 0));
    }
    selected = item;
    int index = item->listWidget()->row(item);
    item->setBackground(QColor(14,193,251,100));
    playlist->setCurrentIndex(index);
    player->play();
}

void MainWindow::ChangeMedia(QMediaContent media) {
    if(player->state() == QMediaPlayer::StoppedState) return;
    if(selected != NULL && !media.isNull()) {
        selected->setBackground(QColor(0, 0, 0, 0));
        selected = ui->list_song->item(playlist->currentIndex());
        selected->setBackground(QColor(14,193,251,100));
    }
}

void MainWindow::SavePlaylist() {
    if(playlist->mediaCount() < 1) {
        QMessageBox::warning(this, "Erreur", "La liste de lecture est vide, ajoutez des fichiers audios avant d'enregistrer.");
        return;
    }
    QString filename = QFileDialog::getSaveFileName(this, "Enregistrer la playlist", QStandardPaths::writableLocation(QStandardPaths::MusicLocation), "XSPF Files (*.xspf)");
    if(filename.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Nom de fichier invalide.");
        return;
    }
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    filename = filename.section("/", -1);
    out << "<?xml version=\"1.0\" enconding=\"utf-8\">\n";
    out << "<playlist xmlns=\"http://xspf.org/ns/0/\" version=\"1\">\n";
    out << "<title>" + filename + "</title>\n";
    out << "<tracklist>\n";

    for(int i = 0; i < musicPath->count(); i++) {
        QString title = ui->list_song->item(i)->text();
        int pos;
        for(int j = 0; j < title.length(); j++) {
            if(title[j] == QChar('.')) pos = j;
        }
        title.remove(pos, 5);
        QString path = musicPath->value(i);
        for(int y = 0; y < path.length(); y++) {
            if(path[y] == ' ') path[y] = '%';
        }
        QString prefixe;
        if(path[0] == '/') prefixe = "file://";
        else prefixe = "file:///";
        out << "\t<track><location>" + prefixe + path + "</location><title>" + title + "</title></track>\n";
    }
    out << "</tracklist>";
    QMessageBox::information(this, "Succes", "La liste de lecture " + filename + " a été enregistrée avec succès.");
}

void MainWindow::LoadPlaylist() {
    QString filename = QFileDialog::getOpenFileName(this, "Charger une liste de lecture", QStandardPaths::writableLocation(QStandardPaths::MusicLocation), "XSPF Files (*.xspf)");
    if(filename.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Fichier invalide.");
        return;
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream read(&file);
    QRegularExpressionMatch match;
    QRegularExpression regexp("<location>(.*)</location>", QRegularExpression::CaseInsensitiveOption);
    while(!read.atEnd()) {
        QString line = read.readLine().trimmed();
        match = regexp.match(line);
        if(match.hasMatch()) {
            QString captured = match.captured(1);
            if (!captured.contains("file:///")) {
                if(captured[0] == '/') captured = "file://" + captured;
                else captured = "file:///" + captured;
            }
            /*for(int y = 0; y < captured.length(); y++) {
                if(captured[y] == '%') captured[y] = ' ';
            }*/
            PlaylistAdd(QMediaContent(captured));
            qDebug() << QMediaContent(captured).canonicalUrl().toString();
        }
    }
    filename = filename.section("/", -1);
    QMessageBox::information(this, "Succes", "La liste de lecture " + filename + " a été chargée avec succès.");

}

void MainWindow::SetTimer() {
    if(timer->isActive()) {
        QMessageBox::information(this, "Succes", "Le timer a été annulé.");
        timer->stop();
        return;
    }

    int time = QInputDialog::getInt(this, "Timer", "Insérez le nombre de minutes qui doivent s'écouler avant que la musique ne coupe.", 1, 1);
    if(time < 1) QMessageBox::warning(this, "Erreur", "Merci d'indiquer une valeur supérieure à zéro.");
    timer->start(time*60*1000);
}

void MainWindow::EndTimer() {
    player->pause();
    QMessageBox::information(this, "Fin du timer", "Le timer a expiré à " + QDateTime::currentDateTime().toString("H:m:s"));
}
