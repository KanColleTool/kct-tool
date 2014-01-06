#include "KCMainWindow.h"
#include "ui_KCMainWindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include "KCShip.h"
#include "KCShipMaster.h"
#include "KCDock.h"
#include "KCMacUtils.h"

KCMainWindow::KCMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::KCMainWindow),
	apiLinkDialogOpen(false)
{
	ui->setupUi(this);
	
	this->_setupClient();
	this->_setupTrayIcon();
	this->_setupUI();
	
	// Set the Fleets page regardless of what the UI file says.
	// (This saves me from accidentally releasing a version with the wrong
	// start page due to me editing another one right beforehand)
	this->on_actionFleets_triggered();
}

KCMainWindow::~KCMainWindow()
{
	delete ui;
}

void KCMainWindow::_setupTrayIcon()
{
	// Create the Tray Icon
	this->trayIcon = new QSystemTrayIcon(QIcon(":/icon-48x48.png"), this);
	connect(this->trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
	this->trayIcon->show();
	
	// Set up the menu for it, but not if we're on a Mac.
	// On Mac, it's more convenient to have a click bring up the main window
	// (since left-click also brings up the menu there)
#ifndef __APPLE__
	this->trayMenu = new QMenu("KanColleTool", this);
	this->trayMenu->addAction("Show", this, SLOT(showApplication()));
	this->trayMenu->addAction("Exit", qApp, SLOT(quit()));
	this->trayIcon->setContextMenu(this->trayMenu);
#endif
}

void KCMainWindow::_setupUI()
{
	// Right-align some items on the toolbar
	QWidget *toolbarSpacer = new QWidget();
	toolbarSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	ui->toolBar->insertWidget(ui->actionRefresh, toolbarSpacer);
	
	// Set up the Fleets page
	ui->fleetsTabBar->addTab("Fleet 1");
	ui->fleetsTabBar->addTab("Fleet 2");
	ui->fleetsTabBar->addTab("Fleet 3");
	ui->fleetsTabBar->addTab("Fleet 4");
	
	// On Mac, make the window join all spaces
	// (why isn't there a Qt call for this...)
#ifdef __APPLE__
	macSetWindowOnAllWorkspaces(this);
#endif
}

void KCMainWindow::_setupClient()
{
	this->client = new KCClient(this);
	
	connect(this->client, SIGNAL(credentialsGained()), this, SLOT(onCredentialsGained()));
	connect(this->client, SIGNAL(receivedMasterShips()), this, SLOT(onReceivedMasterShips()));
	connect(this->client, SIGNAL(receivedPlayerShips()), this, SLOT(onReceivedPlayerShips()));
	connect(this->client, SIGNAL(receivedPlayerFleets()), this, SLOT(onReceivedPlayerFleets()));
	connect(this->client, SIGNAL(receivedPlayerRepairs()), this, SLOT(onReceivedPlayerRepairs()));
	connect(this->client, SIGNAL(requestError(KCClient::ErrorCode)), this, SLOT(onRequestError(KCClient::ErrorCode)));
	
	if(!this->client->hasCredentials())
		this->askForAPILink();
	else
		this->onCredentialsGained();
}

bool KCMainWindow::isApplicationActive()
{
#ifdef __APPLE__
	return (macApplicationIsActive() && this->isVisible());
#else
	return this->hasFocus();
#endif
}

void KCMainWindow::toggleApplication()
{
	if(!this->isApplicationActive())
		this->showApplication();
	else
		this->hideApplication();
}

void KCMainWindow::showApplication()
{
#ifdef __APPLE__
	macApplicationActivate();
	this->show();
#else
	this->show();
	this->setFocus();
	this->activateWindow();
	this->raise();
#endif
}

void KCMainWindow::hideApplication()
{
	this->hide();
}

void KCMainWindow::askForAPILink()
{
	if(apiLinkDialogOpen)
		return;
	
	apiLinkDialogOpen = true;
	QUrl url = QInputDialog::getText(this, "Enter API Link", "<p>Enter your API Link.<br />It should look something like:</p><p><code>http://125.6.189.xxx/kcs/mainD2.swf?api_token=xxxxxxxxxxxxxxxxxxxx...</code></p>");
	QUrlQuery query(url);
	apiLinkDialogOpen = false;
	
	this->client->setCredentials(url.host(), query.queryItemValue("api_token"));
}

void KCMainWindow::updateFleetsPage()
{
	ui->fleetsPage->setUpdatesEnabled(false);
	
	// Hide all the boxes by default, then show the ones we use below
	for(int i = 0; i < 6; i++)
		findChild<QGroupBox*>(QString("fleetBox") + QString::number(i+1))->hide();
	
	// If there is no such fleet, return here and leave all boxes hidden
	if(!client->fleets.contains(ui->fleetsTabBar->currentIndex()+1))
		return;
	
	// Otherwise, retreive it
	KCFleet *fleet = client->fleets[ui->fleetsTabBar->currentIndex()+1];
	
	// Loop through all the ships in the fleet and put their info up
	for(int i = 0; i < fleet->shipCount; i++)
	{
		KCShip *ship = client->ships[fleet->ships[i]];
		if(!ship) continue;
		
		QString iS = QString::number(i+1);
		
		QGroupBox *box = findChild<QGroupBox*>(QString("fleetBox") + iS);
		QLabel *nameLabel = findChild<QLabel*>(QString("fleetName") + iS);
		QLabel *readingLabel = findChild<QLabel*>(QString("fleetReading") + iS);
		QProgressBar *hpBar = findChild<QProgressBar*>(QString("fleetHpBar") + iS);
		QProgressBar *ammoBar = findChild<QProgressBar*>(QString("fleetAmmoBar") + iS);
		QProgressBar *fuelBar = findChild<QProgressBar*>(QString("fleetFuelBar") + iS);
		QLabel *levelLabel = findChild<QLabel*>(QString("fleetLevel") + iS);
		QLabel *condLabel = findChild<QLabel*>(QString("fleetCond") + iS);
		
		box->show();
		nameLabel->setText(ship->name);
		readingLabel->setText(ship->reading);
		hpBar->setRange(0, ship->maxHp);
		hpBar->setValue(ship->hp);
		ammoBar->setRange(0, ship->maxAmmo);
		ammoBar->setValue(ship->ammo);
		fuelBar->setRange(0, ship->maxFuel);
		fuelBar->setValue(ship->fuel);
		levelLabel->setText(QString::number(ship->level));
		condLabel->setText(QString::number(ship->condition));
	}
	
	ui->fleetsPage->setUpdatesEnabled(true);
}

void KCMainWindow::updateShipsPage()
{
	ui->shipsPage->setUpdatesEnabled(false);
	
	ui->shipsTable->setSortingEnabled(false);
	ui->shipsTable->setRowCount(client->ships.count());
	
	int row = 0;
	foreach(KCShip *ship, client->ships)
	{
		ui->shipsTable->setItem(row, 0, new QTableWidgetItem(QString::number(ship->level)));
		ui->shipsTable->setItem(row, 1, new QTableWidgetItem(QString::number(ship->maxHp)));
		ui->shipsTable->setItem(row, 2, new QTableWidgetItem(QString::number(ship->firepower.cur)));
		ui->shipsTable->setItem(row, 3, new QTableWidgetItem(QString::number(ship->torpedo.cur)));
		ui->shipsTable->setItem(row, 4, new QTableWidgetItem(QString::number(ship->evasion.cur)));
		ui->shipsTable->setItem(row, 5, new QTableWidgetItem(QString::number(ship->antiair.cur)));
		ui->shipsTable->setItem(row, 6, new QTableWidgetItem(QString::number(ship->antisub.cur)));
		ui->shipsTable->setItem(row, 7, new QTableWidgetItem(QString::number(ship->speed)));
		ui->shipsTable->setItem(row, 8, new QTableWidgetItem(QString("%1 (%2)").arg(ship->name, ship->reading)));
		
		++row;
	}
	
	ui->shipsTable->setSortingEnabled(true);
	
	ui->shipsPage->setUpdatesEnabled(true);
}

void KCMainWindow::updateRepairsPage()
{
	ui->repairsPage->setUpdatesEnabled(false);
	
	int i = 0;
	foreach(KCDock *dock, client->repairDocks)
	{
		QString iS = QString::number(i+1);
		QGroupBox *box = findChild<QGroupBox*>(QString("repairBox") + iS);
		QLabel *nameLabel = findChild<QLabel*>(QString("repairName") + iS);
		QLabel *readingLabel = findChild<QLabel*>(QString("repairReading") + iS);
		QLabel *repairTimerLabel = findChild<QLabel*>(QString("repairTimer") + iS);
		
		qDebug() << "Dock" << i << dock->state << dock->shipID;
		
		if(dock->state == KCDock::Locked)
		{
			box->setEnabled(false);
			nameLabel->setText("(Locked)");
			readingLabel->setText("");
			repairTimerLabel->setText("");
		}
		else if(dock->state == KCDock::Empty)
		{
			box->setEnabled(true);
			nameLabel->setText("(Empty)");
			readingLabel->setText("");
			repairTimerLabel->setText("0:00:00");
		}
		else if(dock->state == KCDock::Occupied)
		{
			box->setEnabled(true);
			KCShip *ship = client->ships[dock->shipID];
			qDebug() << "Ship:" << ship;
			if(!ship)
				continue;
			
			nameLabel->setText(ship->name);
			readingLabel->setText(ship->reading);
			repairTimerLabel->setText(dock->complete.toString("HH:mm:ss"));
		}
		
		++i;
	}
	
	ui->repairsPage->setUpdatesEnabled(true);	
}

void KCMainWindow::onCredentialsGained()
{
	qDebug() << "Credentials Gained";
	this->client->requestMasterShips();
	this->client->requestPlayerShips();
	this->client->requestPlayerFleets();
	this->client->requestPlayerRepairs();
}

void KCMainWindow::onReceivedMasterShips()
{
	qDebug() << "Received Master Ship Data" << client->masterShips.size();
}

void KCMainWindow::onReceivedPlayerShips()
{
	qDebug() << "Received Player Ship Data" << client->ships.size();
	updateFleetsPage();
	updateShipsPage();
	updateRepairsPage();
}

void KCMainWindow::onReceivedPlayerFleets()
{
	qDebug() << "Received Player Fleet Data" << client->fleets.size();
	updateFleetsPage();
}

void KCMainWindow::onReceivedPlayerRepairs()
{
	qDebug() << "Received Player Repairs Data" << client->repairDocks.size();
	updateRepairsPage();
}

void KCMainWindow::onRequestError(KCClient::ErrorCode error)
{
	switch(error)
	{
		case KCClient::JsonError:
			QMessageBox::warning(this, "JSON Error", "The response was malformed JSON and could not be parsed. This could mean that there's something messing with your internet connection.");
			break;
		case KCClient::InvalidAPIVersion:
			QMessageBox::critical(this, "Invalid API Version", "KanColle changed their API, and this program is outdated.");
			qApp->quit();
			break;	// OCD
		case KCClient::InvalidCredentials:
			this->askForAPILink();
			break;
		default:
			QMessageBox::warning(this, "Unknown Error", "An unknown error occurred.");
	}
}

void KCMainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
#ifdef __APPLE__
	Q_UNUSED(reason);
	this->toggleApplication();
#else
	if(reason != QSystemTrayIcon::Context)
		this->toggleApplication();
#endif
}

void KCMainWindow::on_actionFleets_triggered()
{
	ui->actionFleets->setEnabled(false);
	ui->actionShips->setEnabled(true);
	ui->actionRepairs->setEnabled(true);
	ui->actionConstruction->setEnabled(true);
	ui->stackedWidget->setCurrentWidget(ui->fleetsPage);
}

void KCMainWindow::on_actionShips_triggered()
{
	ui->actionFleets->setEnabled(true);
	ui->actionShips->setEnabled(false);
	ui->actionRepairs->setEnabled(true);
	ui->actionConstruction->setEnabled(true);
	ui->stackedWidget->setCurrentWidget(ui->shipsPage);
}

void KCMainWindow::on_actionRepairs_triggered()
{
	ui->actionFleets->setEnabled(true);
	ui->actionShips->setEnabled(true);
	ui->actionRepairs->setEnabled(false);
	ui->actionConstruction->setEnabled(true);
	ui->stackedWidget->setCurrentWidget(ui->repairsPage);
}

void KCMainWindow::on_actionConstruction_triggered()
{
	ui->actionFleets->setEnabled(true);
	ui->actionShips->setEnabled(true);
	ui->actionRepairs->setEnabled(true);
	ui->actionConstruction->setEnabled(false);
	ui->stackedWidget->setCurrentWidget(ui->constructionPage);
}

void KCMainWindow::on_actionRefresh_triggered()
{
    client->requestPlayerShips();
	client->requestPlayerFleets();
	client->requestPlayerRepairs();
}

void KCMainWindow::on_fleetsTabBar_currentChanged(int index)
{
	//qDebug() << "Fleets page on Tab" << index;
	Q_UNUSED(index);
	updateFleetsPage();
}
