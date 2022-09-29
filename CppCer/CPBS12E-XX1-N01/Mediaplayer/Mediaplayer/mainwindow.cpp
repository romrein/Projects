/*******************************************************************
* Einsendeaufgabe
* Datei: mainwindow.cpp
* Heft: 12E
* Student: Wjatscheslaw Rein
* Vertrags-Nr.: 800463563
* Lehrgangs-Nr.: 246
********************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //der horizontale Slider zum Einstellen der Position
    positionsSlider = new QSlider(Qt::Horizontal, this);
    positionsSlider->setMaximumWidth(200);
    positionsSlider->setRange(0, 0);
    positionsSlider->setToolTip("Position ändern");
    positionsSlider->setValue(0);
    ui->statusBar->addWidget(positionsSlider);

    //das Label zur Anzeige der Zeit
    positionsLabel = new QLabel("00:00:00 von 00:00:00", this);
    ui->statusBar->addWidget(positionsLabel);

    //der horizontale Slider zum Einstellen der Lautstärke
    lautstaerkeSlider = new QSlider(Qt::Horizontal, this);
    lautstaerkeSlider->setMaximumWidth(100);
    lautstaerkeSlider->setRange(0, 100);
    lautstaerkeSlider->setToolTip("Lautstärke ändern");
    lautstaerkeSlider->setValue(100);

    ui->toolBarLautstaerke->addWidget(lautstaerkeSlider);

    //das Drehfeld zum Verändern der Lautstärke
    QSpinBox *lautstaerkeSpinBox = new QSpinBox(this);
    lautstaerkeSpinBox->setRange(0, 100);
    lautstaerkeSpinBox->setToolTip("Lautstärke ändern");
    lautstaerkeSpinBox->setValue(100);
    ui->toolBarLautstaerke->addWidget(lautstaerkeSpinBox);

    //Drehfeld und Slider synchronisieren sich wechselseitig
    //bitte in einer Zeile eingeben
    connect(lautstaerkeSlider, SIGNAL(valueChanged(int)), lautstaerkeSpinBox, SLOT(setValue(int)));
    //bitte in einer Zeile eingeben
    connect(lautstaerkeSpinBox, SIGNAL(valueChanged(int)), lautstaerkeSlider, SLOT(setValue(int)));

    this->setWindowTitle("Multimedia-Player");
    //für den MediaPlayer
    meinPlayer = new QMediaPlayer(this);
    //mit dem VideoWidget im Formular verbinden
    meinPlayer->setVideoOutput(ui->videoWidget);
    connect(meinPlayer, SIGNAL(mediaChanged(QMediaContent)), this, SLOT(listeUndTitelAktualisieren()));

    connect(lautstaerkeSlider, SIGNAL(valueChanged(int)), this, SLOT(lautstaerkeAendern(int)));

    connect(positionsSlider, SIGNAL(sliderMoved(int)), this, SLOT(positionAendern(int)));

    connect(meinPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(dauerGeaendert(qint64)));

    connect(meinPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(positionGeaendert(qint64)));

    kontextmenuErzeugen();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*******************************************************************
* void MainWindow::listeUndTitelAktualisieren()
*
* Aufgabe 3
* Funktion: Der Slot für das Laden der Datei in die Wiedergabeliste.
*           Dabei wird sichergestellt, dass nur die Dateien geladen
*           werden die nicht in der Liste vorhanden sind.
********************************************************************/
void MainWindow::listeUndTitelAktualisieren()
{
    QList<QListWidgetItem*> meineListe;
    //den Namen beschaffen
    QString titel = meinPlayer->media().request().url().toLocalFile();

    //Überprüfen ob zuladende Datei bereits in der Wiedergabeliste
    //vorkommt. Eine Such-Methode vergleicht die Einträge auf
    //Übereinstimmung, in unserem Fall volle (MatchExactly).
    meineListe = ui->listWidget->findItems(titel, Qt::MatchExactly);

    //Wenn der Zähler der Liste gleich Null ist, wurde keinen Eintrag
    //gefunden, der den Suchkriterien entspricht. Und die Datei kann
    //geladen werden.
    if (meineListe.count() == 0)
    {
        //den Namen in das Listenfeld schreiben
        ui->listWidget->addItem(titel);
        //den Titel setzen
        this->setWindowTitle("Multimedia-Player – " + titel);
    }
    else
    {
        //Die Datei bereits geladen
    }
}

/*******************************************************************
* void MainWindow::on_action_Beenden_triggered()
*
* Funktion: Schließen des Hauptfensters
********************************************************************/
void MainWindow::on_action_Beenden_triggered()
{
    this->close();
}

/*******************************************************************
* void MainWindow::on_action_ffnen_triggered()
*
* Funktion: Öffnen und Abspielen einer Media-Datei
********************************************************************/
void MainWindow::on_action_ffnen_triggered()
{
    //den Dialog erzeugen
    QFileDialog *meinDialog = new QFileDialog(this);
    //wir brauchen einen Öffnen-Dialog
    meinDialog->setAcceptMode(QFileDialog::AcceptOpen);
    meinDialog->setWindowTitle("Datei öffnen");
    //Filter setzen
    meinDialog->setNameFilters(QStringList() << "Videos (*.mp4 *.wmv)" << "Audios (*.mp3)" << "Alle Dateien (*.*)");
    //den Standardordner setzen
    meinDialog->setDirectory(QDir::currentPath());
    //es soll nur eine Datei ausgewählt werden können
    meinDialog->setFileMode(QFileDialog::ExistingFile);
    //den Dialog öffnen
    //wenn er über die Schaltfläche Öffnen verlassen wird, laden wir die Datei
    if (meinDialog->exec() == QDialog::Accepted)
    {
        //den ersten Eintrag aus der Liste mit den Dateien beschaffen
        QString datei = meinDialog->selectedFiles().first();
        //und laden
        meinPlayer->setMedia(QUrl::fromLocalFile(datei));
        //und wiedergeben
        meinPlayer->play();
    }
    ui->actionTon_ein_aus->setEnabled(true);
    ui->action_Pause->setEnabled(true);
    ui->action_Stopp->setEnabled(true);
}

/*******************************************************************
* void MainWindow::on_action_Pause_triggered()
*
* Funktion: Zusammengefasste Aktionen Pause und Wiedergabe sowie
*           die entsprechende Einstellung Pause/Wiedergabe Symbole
********************************************************************/
void MainWindow::on_action_Pause_triggered()
{
    //ist die Wiedergabe aktuell unterbrochen?
    if (meinPlayer->state() == QMediaPlayer::PausedState)
    {
        //dann zeigen wir das Pausesymbol an und setzen die
        //Wiedergabe fort
        ui->action_Pause->setIcon(QIcon(":/images/pause.png"));
        ui->action_Pause->setToolTip("Pause");
        ui->action_Pause->setText("Pause");
        meinPlayer->play();
    }
    else if (meinPlayer->state() == QMediaPlayer::PlayingState)
    {
        //dann zeigen wir das Wiedergabesymbol an und unterbrechen
        //die Wiedergabe
        ui->action_Pause->setIcon(QIcon(":/images/play.png"));
        ui->action_Pause->setToolTip("Wiedergeben");
        ui->action_Pause->setText("Wiedergeben");
        meinPlayer->pause();
    }
    else if (meinPlayer->state() == QMediaPlayer::StoppedState)
    {
        ui->action_Pause->setIcon(QIcon(":/images/play.png"));
        ui->action_Pause->setToolTip("Wiedergeben");
        ui->action_Pause->setText("Wiedergeben");
        meinPlayer->pause();
    }

}

/*******************************************************************
* void MainWindow::on_actionW_iedergeben_triggered()
*
* Funktion: inaktiv wegen "on_action_Pause_triggered()"
********************************************************************/
void MainWindow::on_actionW_iedergeben_triggered()
{
    meinPlayer->play();
}

/*******************************************************************
* void MainWindow::on_action_Stopp_triggered()
*
* Funktion: Wiedergabe stoppen und die Pause/Wiedergabe Symbol
*           entsprechend einstellen
********************************************************************/
void MainWindow::on_action_Stopp_triggered()
{
    meinPlayer->stop();
    if (meinPlayer->state() == QMediaPlayer::StoppedState)
    {
        ui->action_Pause->setIcon(QIcon(":/images/play.png"));
        ui->action_Pause->setToolTip("Wiedergeben");
        ui->action_Pause->setText("Wiedergeben");
        meinPlayer->pause();
    }
}

/*******************************************************************
* void MainWindow::on_actionTon_ein_aus_triggered()
*
* Aufgabe 2
* Funktion: Lautstärke über Symbol Ein- bzw. Ausschalten dabei wird
*           die aktuelle Value in iLautTmp gespeichert und bei der
*           Wiedereinschalten die Laustärke auf gesicherte Wert
*           zurückgesetzt.
********************************************************************/
void MainWindow::on_actionTon_ein_aus_triggered()
{
    //ist die Lautstärke ausgeschaltet?
    if (meinPlayer->isMuted() == true)
    {
        //dann einschalten
        meinPlayer->setMuted(false);
        ui->actionTon_ein_aus->setIcon(QIcon(":/images/mute.png"));

        //Aufgabe 2: Lautstärke wieder auf gespeicherte Wert setzen
        lautstaerkeSlider->setValue(iLautTmp);
    }
    //sonst ausschalten
    else
    {
        meinPlayer->setMuted(true);
        ui->actionTon_ein_aus->setIcon(QIcon(":/images/mute_off.png"));

        //Aufgabe 2: Vor dem Ausschalten die aktuelle Lautstärke speichern
        iLautTmp = lautstaerkeSlider->value();
        lautstaerkeSlider->setValue(0);
    }
}

/*******************************************************************
* void MainWindow::lautstaerkeAendern(int neueLautstarke)
* par[in]: int neueLautstarke
*
* Aufgabe 2
* Funktion: der Slot für das Verändern der Lautstärke
*           und entsprechende Anpassung der Symbole
********************************************************************/
void MainWindow::lautstaerkeAendern(int neueLautstarke)
{
    meinPlayer->setVolume(neueLautstarke);
    if (neueLautstarke == 0)
    {
        ui->actionTon_ein_aus->setIcon(QIcon(":/images/mute_off.png"));
    }
    else
    {
        ui->actionTon_ein_aus->setIcon(QIcon(":/images/mute.png"));
    }
}

/*******************************************************************
* void MainWindow::on_action_Lautst_rke_triggered()
*
* Aufgabe 1
* Funktion: Die Symbolleiste für die Lautstärke wird ein- bzw.
*           ausgeblendet. Zusätzlich wird bei dem Einschalten die
*           Markierung von "Vollbild"-Eintrag gelöscht und Vollbildmodus
*           ausgeschaltet.
********************************************************************/
void MainWindow::on_action_Lautst_rke_triggered()
{
    //die Symbolleiste Lautstärke ein- bzw. ausblenden
    if (ui->action_Lautst_rke->isChecked())
    {
        ui->toolBarLautstaerke->show();

        //Aufgabe 1: Markierung für den Eintrag Vollbild ändern
        //und Vollbilddarstellung deaktivieren
        ui->action_Vollbild->setChecked(0);
        showNormal();
    }
    else
    {
        ui->toolBarLautstaerke->hide();
    }
}

/*******************************************************************
* void MainWindow::on_actionW_iedergabe_triggered()
*
* Aufgabe 1
* Funktion: Die Symbolleiste für die Wiedergabe wird ein- bzw.
*           ausgeblendet. Zusätzlich wird bei dem Einschalten die
*           Markierung von "Vollbild"-Eintrag gelöscht und Vollbildmodus
*           ausgeschaltet.
********************************************************************/
void MainWindow::on_actionW_iedergabe_triggered()
{
    //die Symbolleiste Wiedergabe ein- bzw. ausblenden
    if (ui->actionW_iedergabe->isChecked())
    {
        ui->toolBarWiedergabe->show();

        //Aufgabe 1: Markierung für den Eintrag Vollbild ändern
        //und Vollbilddarstellung deaktivieren
        ui->action_Vollbild->setChecked(0);
        showNormal();
    }
    else
    {
        ui->toolBarWiedergabe->hide();
    }
}

/*******************************************************************
* void MainWindow::on_action_Wiedergabeliste_triggered()
*
* Aufgabe 1
* Funktion: Die Wiedergabeliste wird ein- bzw. ausgeblendet.
*           Zusätzlich wird bei dem Einschalten die Markierung von
*           "Vollbild"-Eintrag gelöscht und Vollbildmodus ausgeschaltet.
********************************************************************/
void MainWindow::on_action_Wiedergabeliste_triggered()
{
    //die Wiedergabeliste ein- bzw. ausblenden
    if (ui->action_Wiedergabeliste->isChecked())
    {
        ui->dockWidget->show();

        //Aufgabe 1: Markierung für den Eintrag Vollbild ändern
        //und Vollbilddarstellung deaktivieren
        ui->action_Vollbild->setChecked(0);
        showNormal();
    }
    else
    {
        ui->dockWidget->hide();
    }
}

/*******************************************************************
* void MainWindow::on_listWidget_itemClicked (QListWidgetItem* listItem)
* par[in]: QListWidgetItem* listItem
*
* Funktion: Ereignisse beim Anklicken von einem Eintrag in der
*           Wiedergabeliste
********************************************************************/
void MainWindow::on_listWidget_itemClicked (QListWidgetItem* listItem)
{
    //die Datei aus dem markierten Eintrag beschaffen
    QString datei = listItem->text();
    //und laden
    meinPlayer->setMedia(QUrl::fromLocalFile(datei));
    //und wiedergeben
    meinPlayer->play();
    //die Aktionen aktivieren
    ui->actionTon_ein_aus->setEnabled(true);
    ui->action_Pause->setEnabled(true);
    ui->action_Stopp->setEnabled(true);
    //das Symbol für die Pause anzeigen
    ui->action_Pause->setIcon(QIcon(":/images/pause.png"));
    ui->action_Pause->setToolTip("Pause");
    ui->action_Pause->setText("Pause");
}

/*******************************************************************
* void MainWindow::on_actionEintrag_l_schen_triggered()
*
* Funktion: Eintrag in der Wiedergabeliste löschen
********************************************************************/
void MainWindow::on_actionEintrag_l_schen_triggered()
{
    //bitte in einer Zeile eingeben
    ui->listWidget->takeItem(ui->listWidget->row(ui->listWidget->currentItem()));
}

/*******************************************************************
* void MainWindow::on_actionEintr_ge_sortieren_triggered()
*
* Funktion: Einträge in der Wiedergabeliste sortieren
********************************************************************/
void MainWindow::on_actionEintr_ge_sortieren_triggered()
{
    ui->listWidget->sortItems(Qt::AscendingOrder);
}

/*******************************************************************
* void MainWindow::kontextmenuErzeugen()
*
* Funktion: Kontextmenü erzeugen
********************************************************************/
void MainWindow::kontextmenuErzeugen()
{
    ui->listWidget->addAction(ui->actionEintrag_l_schen);
    ui->listWidget->addAction(ui->actionEintr_ge_sortieren);
    ui->listWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
}

/*******************************************************************
* void MainWindow::positionAendern(int neuePosition)
* par[in]: int neuePosition
*
* Funktion: Setzen von einer neuen Position
********************************************************************/
void MainWindow::positionAendern(int neuePosition)
{
    meinPlayer->setPosition(neuePosition);
}

/*******************************************************************
* void MainWindow::dauerGeaendert(qint64 neueDauer)
* par[in]: qint64 neueDauer
*
* Funktion: Berechnung der gesamten Abspielzeit der geladenen Datei
********************************************************************/
void MainWindow::dauerGeaendert(qint64 neueDauer)
{
    positionsSlider->setMaximum(neueDauer);
    //wir brauchen die Zeit in Sekunden
    neueDauer = neueDauer / 1000;
    //ein neues Objekt vom Typ QTime aus den Angaben erzeugen
    QTime tempGesamtZeit((neueDauer / 3600) % 60, (neueDauer / 60) % 60, neueDauer % 60);
    //formatieren und im Attribut gesamtZeit speichern
    gesamtZeit = tempGesamtZeit.toString("hh:mm:ss");
}

/*******************************************************************
* void MainWindow::positionGeaendert(qint64 neuePosition)
* par[in]: qint64 neuePosition
*
* Funktion: Berechnung der aktuellen "Position" auf der Zeitachse
*           von der geladenen Datei
********************************************************************/
void MainWindow::positionGeaendert(qint64 neuePosition)
{
    positionsSlider->setValue(neuePosition);
    //wir brauchen die Zeit in Sekunden
    neuePosition = neuePosition / 1000;
    //ein neues Objekt vom Typ QTime aus den Angaben erzeugen
    QTime aktuelleZeit((neuePosition / 3600) % 60, (neuePosition / 60) % 60, neuePosition % 60);
    //die Angaben im Label zusammenbauen
    positionsLabel->setText(aktuelleZeit.toString("hh:mm:ss") + " von " + gesamtZeit);
}

/*******************************************************************
* void MainWindow::on_actionInfo_triggered()
*
* Funktion: Eine Info-Meldung erzeugen
********************************************************************/
void MainWindow::on_actionInfo_triggered()
{
    QMessageBox::information(this, "Information", "Programmiert von W. Rein>");
}

/*******************************************************************
* void MainWindow::on_action_Vollbild_triggered()
*
* Aufgabe 1
* Funktion: Vollbildmodus ein- bzw. ausschalten. Ereignisse beim
*           Aktivieren einer Vollbilddarstellung. Es werden die beiden
*           Symbolleisten und die Wiedergabeliste ein- bzw. ausgeblendet.
*           Und die entsprechende Markierungen im Menü angepasst.
********************************************************************/
void MainWindow::on_action_Vollbild_triggered()
{
    if (ui->action_Vollbild->isChecked())
    {
       ui->toolBarLautstaerke->hide();
       ui->dockWidget->hide();
       ui->toolBarWiedergabe->hide();
       ui->actionW_iedergabe->setChecked(0);
       ui->action_Lautst_rke->setChecked(0);
       ui->action_Wiedergabeliste->setChecked(0);
       showMaximized();
    }
    else
    {
        ui->toolBarLautstaerke->show();
        ui->dockWidget->show();
        ui->toolBarWiedergabe->show();
        ui->actionW_iedergabe->setChecked(1);
        ui->action_Lautst_rke->setChecked(1);
        ui->action_Wiedergabeliste->setChecked(1);
        showNormal();
    }
}
