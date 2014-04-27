#include "KCMainWindow.h"
#include "ui_KCMainWindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include "KCSettingsDialog.h"
#include "KCTranslator.h"
#include "KCShipType.h"
#include "KCShip.h"
#include "KCDock.h"
#include "KCMacUtils.h"
#include "KCUtil.h"
#include "KCDefaults.h"

#ifdef Q_OS_WIN
	#include <QtWinExtras>
#endif

KCMainWindow::KCMainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::KCMainWindow),
	trayIcon(0), trayMenu(0), client(0), server(0),
	apiLinkDialogOpen(false) {}

bool KCMainWindow::init()
{
	ui->setupUi(this);

	if(!this->_setupServer()) return false;
	this->_setupClient();
	this->_setupTrayIcon();
	this->_setupUI();
	this->_showDisclaimer();

	// Set the right page regardless of what the UI file says.
	// (This saves me from accidentally releasing a version with the wrong
	// start page due to me editing another one right beforehand)
	if(/*QSettings().value("usenetwork", kDefaultUseNetwork).toBool()*/true) {
		this->on_actionFleets_triggered();
	} else {
		ui->stackedWidget->setCurrentWidget(ui->noNetworkPage);
		ui->topContainer->hide();
	}

	// Setup settings and stuff
	connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(on_refreshButton_clicked()));
	this->updateSettingThings();

	// Load the translation
	KCTranslator *tl = KCTranslator::instance();
	connect(tl, SIGNAL(loadFinished()), this, SLOT(onTranslationLoadFinished()));
	connect(tl, SIGNAL(loadFailed(QString)), this, SLOT(onTranslationLoadFailed(QString)));
	tl->loadTranslation();

	// Make a timer that updates the dock timers, with a 1sec interval
	connect(&timerUpdateTimer, SIGNAL(timeout()), this, SLOT(updateTimers()));
	timerUpdateTimer.start(1000);
	updateTimers();	// Don't wait a whole second to update timers

	// Auto-adjust window size and lock it there
	this->adjustSize();
	this->setFixedSize(this->size());

	// Schedule a call to postConstructorSetup
	QTimer::singleShot(0, this, SLOT(postConstructorSetup()));

	return true;
}

KCMainWindow::~KCMainWindow()
{
	delete trayIcon;
	delete trayMenu;
	delete ui;
	delete client;
	delete server;
}

void KCMainWindow::postConstructorSetup()
{
#ifdef Q_OS_WIN
	QtWin::extendFrameIntoClientArea(this->windowHandle(), -1, -1, -1, -1);
	QtWin::enableBlurBehindWindow(this->windowHandle());
#endif
}

QString KCMainWindow::translateName(const QString &name)
{
	if(translation)
		return QString("%1 (%2)").arg(kcTranslate(name), name);
	else
		return name;
}

bool KCMainWindow::_setupServer()
{
	server = new KCToolServer(this);

	if(!server->listen(QHostAddress::LocalHost, 54321)) {
		QNetworkAccessManager qnam(this);
		QNetworkRequest focusReq(QUrl("http://localhost:54321/kctool/focus"));
		focusReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-empty");
		QNetworkReply *reply = qnam.post(focusReq, "");
		QEventLoop loop;
		connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();
		delete reply;

		return false;
	}
	return true;
}

void KCMainWindow::_setupClient()
{
	client = new KCClient(this);
	server->setClient(client);

	connect(client, SIGNAL(focusRequested()), SLOT(showApplication()));
	connect(client, SIGNAL(credentialsGained()), SLOT(on_refreshButton_clicked()));
	connect(client, SIGNAL(receivedAdmiral()), SLOT(onReceivedAdmiral()));
	connect(client, SIGNAL(receivedShipTypes()), SLOT(onReceivedShipTypes()));
	connect(client, SIGNAL(receivedShips()), SLOT(onReceivedShips()));
	connect(client, SIGNAL(receivedFleets()), SLOT(onReceivedFleets()));
	connect(client, SIGNAL(receivedRepairs()), SLOT(onReceivedRepairs()));
	connect(client, SIGNAL(receivedConstructions()), SLOT(onReceivedConstructions()));
	connect(client, SIGNAL(requestError(KCClient::ErrorCode)), SLOT(onRequestError(KCClient::ErrorCode)));
	connect(client, SIGNAL(dockCompleted(KCDock *)), SLOT(onDockCompleted(KCDock *)));
	connect(client, SIGNAL(missionCompleted(KCFleet*)), SLOT(onMissionCompleted(KCFleet*)));

	client->safeShipTypes();

	QSettings settings;
	if(settings.value("usenetwork", kDefaultUseNetwork).toBool()) {
		if(!client->hasCredentials()) {
			this->askForAPILink();

			// Quit if the user pressed Cancel, instead of erroring out
			if(!client->hasCredentials())
				qApp->quit();
		} else {
			qDebug() << "Credentials Gained";
			client->safeShipTypes();
			this->on_refreshButton_clicked();
		}
	}
}

void KCMainWindow::_setupTrayIcon()
{
	// Create the Tray Icon
	trayIcon = new QSystemTrayIcon(QIcon(":/KanColleTool.png"), this);
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
	trayIcon->show();

	// Set up the menu for it, but not if we're on a Mac.
	// On Mac, it's more convenient to have a click bring up the main window
	// (since left-click also brings up the menu there)
#if !defined(__APPLE__)
	trayMenu = new QMenu("KanColleTool", this);
	trayMenu->addAction("Fleets", this, SLOT(on_actionFleets_triggered()));
	trayMenu->addAction("Ships", this, SLOT(on_actionShips_triggered()));
	trayMenu->addAction("Repairs", this, SLOT(on_actionRepairs_triggered()));
	trayMenu->addAction("Construction", this, SLOT(on_actionConstruction_triggered()));
	trayMenu->addSeparator();
	trayMenu->addAction("Show", this, SLOT(showApplication()));
	trayMenu->addAction("Hide", this, SLOT(hideApplication()));
	trayMenu->addAction("Exit", qApp, SLOT(quit()));
	trayIcon->setContextMenu(this->trayMenu);
#endif
}

void KCMainWindow::_setupUI()
{
	// Set up Mac-specific styling
#ifdef Q_OS_MAC
	{
		// Right-align Settings and Refresh on the toolbar
		QWidget *toolbarSpacer = new QWidget();
		toolbarSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		ui->toolBar->insertWidget(ui->actionSettings, toolbarSpacer);
		
		// Remove the tab bar; the toolbar replaces it
		ui->tabBar->hide();
		
		// This doesn't work for some reason; it just blacks the window contents out...
		//this->setAttribute(Qt::WA_MacBrushedMetal, true);
		
		// Set the tab bars to Document Mode, otherwise it looks awful on OSX
		ui->tabBar->setDocumentMode(true);
		ui->fleetsTabBar->setDocumentMode(true);
		
		// Style stuff
		ui->tabBar->layout()->setSpacing(0);
		this->setStyleSheet(
			"#tabBar QToolButton {"				// Make the buttons on the tab bar blend in
			"	border: none;"
			"	background: none;"
			"	color: #111;"
			"	padding-right: 7px;"
			"}"
			"#tabBar QToolButton:pressed {"		// Make them white when clicked
			"	color: #fff;"
			"}"
			"#tabBar QToolButton:disabled {"
			"	color: #555;"
			"}"
			);
		
		// Make the window join all spaces (why isn't there a Qt call for this...)
		macSetWindowOnAllWorkspaces(this);
		
		// Cmd+R (given to Qt as Ctrl+R and remapped) makes more sense for the
		// refresh command than F5 on OSX, where you generally avoid F-keys
		ui->actionRefresh->setShortcut(Qt::CTRL|Qt::Key_R);
	}
#else
	{
		// Only OSX uses the toolbar; hide it everywhere else
		ui->toolBar->hide();
		
		// Add tabs to the tab bar
		ui->tabBar->addTab("Fleets");
		ui->tabBar->addTab("Ships");
		ui->tabBar->addTab("Repairs");
		ui->tabBar->addTab("Construction");
		
		// Set up shortcuts for them
		new QShortcut(QKeySequence("Ctrl+1"), this, SLOT(on_actionFleets_triggered()));
		new QShortcut(QKeySequence("Ctrl+2"), this, SLOT(on_actionShips_triggered()));
		new QShortcut(QKeySequence("Ctrl+3"), this, SLOT(on_actionRepairs_triggered()));
		new QShortcut(QKeySequence("Ctrl+4"), this, SLOT(on_actionConstruction_triggered()));
		
		// On OSX, we get Ctrl+Q for free, on everything else, set it up manually
		QShortcut *quitShortcut = new QShortcut(QKeySequence("Ctrl+Q"), this);
		connect(quitShortcut, SIGNAL(activated()), qApp, SLOT(quit()));
	}
#endif

	// Setup Windows-specific styling
#ifdef Q_OS_WIN
	{
		// Make the window translucent
		this->setAttribute(Qt::WA_NoSystemBackground);
		
		// Style stuff
		this->setStyleSheet(
					"#fleetsContainer, #shipsTable, #repairsPage, #constructionPage {"
					"	background-color: #fff;"
					"	border: 1px solid #999;"
					"	border-top: none;"
					"}"
					"#fleetsContainer { border-bottom: none; }"
					);
	}
#endif



	// Set up the Fleets page
	{
		// Unset the min height; it glitches stuff up
		ui->fleetsTabBar->setMinimumHeight(0);

		// Make the tabs point downwards
		ui->fleetsTabBar->setShape(QTabBar::RoundedSouth);

		// Add a tab for Fleet 1, the only fleet we can be sure is there
		ui->fleetsTabBar->addTab("Fleet 1");
	}

	// Set up the Constructions page
	{
		// Listen for clicks on the spoiler buttons to update the UI
		connect(ui->constructionSpoil1, SIGNAL(toggled(bool)), this, SLOT(updateConstructionsPage()));
		connect(ui->constructionSpoil2, SIGNAL(toggled(bool)), this, SLOT(updateConstructionsPage()));
		connect(ui->constructionSpoil3, SIGNAL(toggled(bool)), this, SLOT(updateConstructionsPage()));
		connect(ui->constructionSpoil4, SIGNAL(toggled(bool)), this, SLOT(updateConstructionsPage()));
	}
}

void KCMainWindow::_showDisclaimer()
{
	QSettings settings;

	// Only show the disclaimer if it has not already been shown
	// Using an int here because I might want to show it again if I make any
	// big changes to it sometime; shouldn't happen though...
	if(settings.value("disclaimerShown", 0).toInt() <= 0)
	{
		QMessageBox::information(this, "Disclaimer",
			"<p>"
#if __APPLE__
			"Disclaimer:"
			"</p>"
			"<p>"
#endif
			"It's important to note that KanColleTool is not a cheat tool.<br /> "
			"It will not let you do anything the game would not usually let you do."
			"</p>"
			"<p>"
			"Using KanColleTool will not increase your chances of getting banned.<br />"
			"If you're a foreign player, that's already reason enough to ban you, but KCT "
			"is impossible to differentiate from a web browser on their end."
			"</p>");
		settings.setValue("disclaimerShown", 1);
	}
}

void KCMainWindow::closeEvent(QCloseEvent *event)
{
	QSettings settings;
	if(settings.value("minimizeToTray", kDefaultMinimizeToTray).toBool())
	{
		// The first time the application is minimized to the tray, display a
		// message to alert the user about this, in case their window manager
		// hides it by default (*cough* Windows 7 *cough*).
		// This doesn't make sense on OSX, because the program is always in the
		// menu bar in the first place there, with no dock icon at all.
#if !defined(__APPLE__)
		if(!settings.value("closeToTrayNotificationShown").toBool())
		{
			trayIcon->showMessage("Still running!", "KanColleTool is still running in the tray.\nYou can disable that in the settings.");
			settings.setValue("closeToTrayNotificationShown", true);
		}
#endif
	}
	else
	{
		// Just quit if we're not set to stay in the tray when closed
		qApp->quit();
	}

	event->accept();
}

void KCMainWindow::keyPressEvent(QKeyEvent *event)
{
	// Only do this on the fleets page
	if(!ui->actionFleets->isEnabled())
	{
		int moveToTab = -1;
		switch(event->key())
		{
			case Qt::Key_1:		moveToTab = 0; break;
			case Qt::Key_2:		moveToTab = 1; break;
			case Qt::Key_3:		moveToTab = 2; break;
			case Qt::Key_4:		moveToTab = 3; break;
			default: break;
		}

		if(moveToTab != -1 && moveToTab < ui->fleetsTabBar->count())
			ui->fleetsTabBar->setCurrentIndex(moveToTab);
	}

	QMainWindow::keyPressEvent(event);
}

bool KCMainWindow::isApplicationActive()
{
#ifdef __APPLE__
	return (macApplicationIsActive() && this->isVisible());
#else
	return QApplication::activeWindow() != 0 || QApplication::focusWidget() != 0;
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

	client->setCredentials(url.host(), query.queryItemValue("api_token"));
}

void KCMainWindow::updateFleetsPage()
{
	ui->fleetsPage->setUpdatesEnabled(false);

	// Hide all the boxes by default, then show the ones we use below
	for(int i = 0; i < 6; i++)
		findChild<QGroupBox*>(QString("fleetBox%1").arg(i+1))->hide();

	// If there is no such fleet, return here and leave all boxes hidden
	if(!client->fleets.contains(ui->fleetsTabBar->currentIndex()+1)) {
		ui->fleetsPage->setUpdatesEnabled(true);
		return;
	}

	// Otherwise, retreive it
	KCFleet *fleet = client->fleets[ui->fleetsTabBar->currentIndex()+1];
	if(fleet)
	{
		// Loop through all the ships in the fleet and put their info up
		for(int i = 0; i < fleet->shipCount; i++)
		{
			KCShip *ship = client->ships[fleet->ships[i]];
			if(!ship) continue;
			KCShipType *type = client->shipTypes[ship->type];

			QString iS = QString::number(i+1);

			QGroupBox *box = findChild<QGroupBox*>(QString("fleetBox") + iS);
			QProgressBar *hpBar = findChild<QProgressBar*>(QString("fleetHpBar") + iS);
			QProgressBar *ammoBar = findChild<QProgressBar*>(QString("fleetAmmoBar") + iS);
			QProgressBar *fuelBar = findChild<QProgressBar*>(QString("fleetFuelBar") + iS);
			QLabel *levelLabel = findChild<QLabel*>(QString("fleetLevel") + iS);
			QLabel *condLabel = findChild<QLabel*>(QString("fleetCond") + iS);
			
			if(ship->hp.cur < ship->hp.max)
				hpBar->setToolTip(QString("Repair Time: %1, Repair Cost: %2 Steel, %3 Fuel").arg(ship->repairTime.toString("H:mm:ss"), QString::number(ship->repairCost.steel), QString::number(ship->repairCost.fuel)));
			else
				hpBar->setToolTip("Healthy");

			box->show();
			hpBar->setRange(0, ship->hp.max);
			hpBar->setValue(ship->hp.cur);
			levelLabel->setText(QString::number(ship->level));
			condLabel->setText(QString::number(ship->condition));

			if(type) {
				box->setTitle(translateName(type->name));
				ammoBar->setRange(0, type->maxAmmo);
				ammoBar->setValue(ship->ammo);
				fuelBar->setRange(0, type->maxFuel);
				fuelBar->setValue(ship->fuel);
			} else {
				box->setTitle("(Loading...)");
				ammoBar->setRange(0, 0);
				fuelBar->setRange(0, 0);
			}
		}
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
		if(!ship) continue;
		KCShipType *type = client->shipTypes[ship->type];

		TABLE_SET_ITEM(ui->shipsTable, row, 0, ship->level);
		TABLE_SET_ITEM(ui->shipsTable, row, 1, ship->hp.max);
		TABLE_SET_ITEM(ui->shipsTable, row, 2, ship->firepower);
		TABLE_SET_ITEM(ui->shipsTable, row, 3, ship->torpedo);
		TABLE_SET_ITEM(ui->shipsTable, row, 4, ship->evasion);
		TABLE_SET_ITEM(ui->shipsTable, row, 5, ship->antiair);
		TABLE_SET_ITEM(ui->shipsTable, row, 6, ship->antisub);
		TABLE_SET_ITEM(ui->shipsTable, row, 7, ship->luck);
		if(type) {
			TABLE_SET_ITEM(ui->shipsTable, row, 8, translateName(type->name));
		} else {
			TABLE_SET_ITEM(ui->shipsTable, row, 8, "(Loading...)");
		}

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
		if(!dock) continue;

		QString iS = QString::number(i+1);
		QGroupBox *box = findChild<QGroupBox*>(QString("repairBox") + iS);
		QLabel *nameLabel = findChild<QLabel*>(QString("repairName") + iS);
		QLabel *repairTimerLabel = findChild<QLabel*>(QString("repairTimer") + iS);

		if(dock->state == KCDock::Locked)
		{
			box->setEnabled(false);
			nameLabel->setText("(Locked)");
			repairTimerLabel->setText("");
		}
		else if(dock->state == KCDock::Empty)
		{
			box->setEnabled(true);
			nameLabel->setText("(Empty)");
			repairTimerLabel->setText("‒:‒‒:‒‒");
		}
		else if(dock->state == KCDock::Occupied)
		{
			box->setEnabled(true);
			KCShip *ship = client->ships[dock->shipID];
			if(ship) {
				KCShipType *type = client->shipTypes[ship->type];
				if(type)
					nameLabel->setText(translateName(type->name));
				else
					nameLabel->setText("(Loading...)");
				repairTimerLabel->setText(delta(dock->complete).toString("H:mm:ss"));
			}
		}
		else qWarning() << "Unknown State for Repair Dock" << i << ":" << dock->state;

		++i;
	}

	ui->repairsPage->setUpdatesEnabled(true);
}

void KCMainWindow::updateConstructionsPage()
{
	// Whee copypaste!

	ui->constructionPage->setUpdatesEnabled(false);

	int i = 0;
	foreach(KCDock *dock, client->constructionDocks)
	{
		if(!dock) continue;

		QString iS = QString::number(i+1);
		QGroupBox *box = findChild<QGroupBox*>(QString("constructionBox") + iS);
		QLabel *nameLabel = findChild<QLabel*>(QString("constructionName") + iS);
		QLabel *buildTimerLabel = findChild<QLabel*>(QString("constructionTimer") + iS);
		QCheckBox *spoilCheckbox = findChild<QCheckBox*>(QString("constructionSpoil") + iS);

		if(dock->state == KCDock::Locked)
		{
			box->setEnabled(false);
			nameLabel->setText("(Locked)");
			buildTimerLabel->setText("");
			spoilCheckbox->hide();
		}
		else if(dock->state == KCDock::Empty)
		{
			box->setEnabled(true);
			nameLabel->setText("(Empty)");
			buildTimerLabel->setText("‒:‒‒:‒‒");
			spoilCheckbox->hide();
			spoilCheckbox->setChecked(false);	// Uncheck it!
		}
		else if(dock->state == KCDock::Occupied || dock->state == KCDock::Building || dock->state == KCDock::Finished)
		{
			box->setEnabled(true);

			if(spoilCheckbox->isChecked())
			{
				KCShipType *ship = client->shipTypes[dock->shipID];
				if(ship)
					nameLabel->setText(translateName(ship->name));
				else
					nameLabel->setText("(Loading...)");
			}
			else
				nameLabel->setText("???");

			if(dock->state == KCDock::Occupied || dock->state == KCDock::Building)
				buildTimerLabel->setText(delta(dock->complete).toString("H:mm:ss"));
			else
				buildTimerLabel->setText("0:00:00");
			spoilCheckbox->show();
		}
		else qWarning() << "Unknown State for Construction Dock" << i << ":" << dock->state;

		++i;
	}

	ui->constructionPage->setUpdatesEnabled(true);
}

void KCMainWindow::updateTimers()
{
	// Fleet Status
	{
		KCFleet *fleet = client->fleets[ui->fleetsTabBar->currentIndex()+1];

		// Second condition is a hack to make sure to wait for updateFleetsPage()
		if(fleet && !ui->fleetBox1->isHidden())
		{
			ui->fleetStatus->show();

			bool busy = false;
			QString status = "Combat-Ready";
			QTime dT;

			// Check if the fleet is out on an expedition
			if(fleet->mission.page > 0 && fleet->mission.no > 0 && fleet->mission.complete > QDateTime::currentDateTime())
			{
				busy = true;
				status = QString("Doing Expedition %1-%2").arg(fleet->mission.page).arg(fleet->mission.no);
				dT = delta(fleet->mission.complete);
			}

			// Check if anyone is in the bath; you never disturb a lady who's
			// taking a bath, not even if an Airfield Hime invades the base
			foreach(KCDock *dock, client->repairDocks)
			{
				// Skip already done or empty (completion time = Epoch+0ms) docks
				if(dock->complete < QDateTime::currentDateTime())
					continue;

				for(int i = 0; i < fleet->shipCount; i++)
				{
					if(fleet->ships[i] == dock->shipID)
					{
						// Make sure to use the longest reppair countdown
						QTime dT2 = delta(dock->complete);
						if(dT2 < dT)
							continue;

						KCShip *ship = client->ships[fleet->ships[i]];
						KCShipType *type = client->shipTypes[ship->type];
						busy = true;
						if(type)
							status = QString("%1 is taking a bath").arg(translateName(type->name));
						else
							status = "(Loading...) is taking a bath";
						dT = dT2;
					}
				}
			}

			// Show it all
			if(busy)
			{
				ui->fleetStatus->setText(status);
				ui->fleetCountdown->setText(dT.toString("H:mm:ss"));
				ui->fleetCountdownContainer->show();
			}
			else
			{
				ui->fleetStatus->setText(status);
				ui->fleetCountdownContainer->hide();
			}
		}
		else
		{
			ui->fleetStatus->hide();
			ui->fleetCountdownContainer->hide();
		}
	}

	// Repair Docks
	{
		int i = 0;
		foreach(KCDock *dock, client->repairDocks)
		{
			if(!dock) continue;

			if(dock->state == KCDock::Occupied)
			{
				QLabel *label = findChild<QLabel*>(QString("repairTimer%1").arg(i+1));
				label->setText(delta(dock->complete).toString("H:mm:ss"));
			}
			++i;
		}
	}

	// Construction Docks
	{
		int i = 0;
		foreach(KCDock *dock, client->constructionDocks)
		{
			if(!dock) continue;

			if(dock->state == KCDock::Building)
			{
				QLabel *label = findChild<QLabel*>(QString("constructionTimer%1").arg(i+1));
				label->setText(delta(dock->complete).toString("H:mm:ss"));
			}
			++i;
		}
	}
}

void KCMainWindow::updateSettingThings()
{
	QSettings settings;

	// Translation
	if(this->translation != settings.value("toolTranslation", kDefaultTranslation).toBool())
	{
		this->translation = !this->translation;
		this->updateFleetsPage();
		this->updateShipsPage();
		this->updateRepairsPage();
		this->updateConstructionsPage();
	}

	// Server for Viewer data livestreaming
	server->enabled = settings.value("livestream", kDefaultLivestream).toBool();

	// Enable manual reloads
	useNetwork = settings.value("usenetwork", kDefaultUseNetwork).toBool();
	ui->actionRefresh->setEnabled(useNetwork);
	ui->refreshButton->setEnabled(useNetwork);

	// Don't remain on the "Livestreaming Only" page if it's disabled!
	if(useNetwork && ui->stackedWidget->currentWidget() == ui->noNetworkPage) {
		leaveNoNetworkPage();
		on_refreshButton_clicked();
	}

	// Autorefreshing (if manual reloads are enabled)
	if(useNetwork && settings.value("autorefresh", kDefaultAutorefresh).toBool())
		refreshTimer.start(settings.value("autorefreshInterval", kDefaultAutorefreshInterval).toInt()*1000);
	else
		refreshTimer.stop();
}

void KCMainWindow::leaveNoNetworkPage()
{
	if(ui->stackedWidget->currentWidget() == ui->noNetworkPage)
	{
		this->setUpdatesEnabled(false);
		ui->topContainer->show();
		this->on_actionFleets_triggered();
		this->setUpdatesEnabled(true);
	}
}

void KCMainWindow::onTranslationLoadFinished()
{
	qDebug() << "Received Translation Data!";
	// Update all the things!
	updateFleetsPage();
	updateShipsPage();
	updateRepairsPage();
	updateConstructionsPage();
}

void KCMainWindow::onTranslationLoadFailed(QString error)
{
	qDebug() << "Failed to load Translation Data..." << error;
	QMessageBox::StandardButton button = QMessageBox::warning(this, "Couldn't load Translation", "You may choose to continue without translation data, but everything will be in Japanese.", QMessageBox::Ok|QMessageBox::Retry, QMessageBox::Retry);
	if(button == QMessageBox::Retry)
		KCTranslator::instance()->loadTranslation();
}

void KCMainWindow::onReceivedAdmiral()
{
	qDebug() << "Received Admiral Data" << client->admiral->nickname;
	updateFleetsPage();
	updateShipsPage();
	updateRepairsPage();
	updateConstructionsPage();
}

void KCMainWindow::onReceivedShipTypes()
{
	qDebug() << "Received Master Ship Data" << client->shipTypes.size();
	updateFleetsPage();
	updateShipsPage();
	updateRepairsPage();
	updateConstructionsPage();
}

void KCMainWindow::onReceivedShips()
{
	qDebug() << "Received Player Ship Data" << client->ships.size();
	updateFleetsPage();
	updateShipsPage();
	updateRepairsPage();
	leaveNoNetworkPage();
}

void KCMainWindow::onReceivedFleets()
{
	qDebug() << "Received Player Fleet Data" << client->fleets.size();

	// If we don't have enough tabs, add some more
	for(int i = ui->fleetsTabBar->count(); i < client->fleets.size(); i++)
		ui->fleetsTabBar->addTab(QString("Fleet %1").arg(i + 1));

	// If we're on an active tab, update it
	if(ui->fleetsTabBar->currentIndex() < client->fleets.size())
	{
		updateFleetsPage();
		updateTimers();
	}

	leaveNoNetworkPage();
}

void KCMainWindow::onReceivedRepairs()
{
	qDebug() << "Received Player Repairs Data" << client->repairDocks.size();
	updateRepairsPage();
	leaveNoNetworkPage();
}

void KCMainWindow::onReceivedConstructions()
{
	qDebug() << "Received Player Constructions Data" << client->constructionDocks.size();
	updateConstructionsPage();
	leaveNoNetworkPage();
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

void KCMainWindow::onDockCompleted(KCDock *dock)
{
	if(dock->isConstruction)
	{
		KCShipType *type = client->shipTypes[dock->shipID];

		// Only name the ship if the player has asked for a spoiler already
		bool spoil = false;
		for(int i = 0; i < client->constructionDocks.size(); i++)
			if(client->constructionDocks[i] == dock)
				spoil = findChild<QCheckBox*>(QString("constructionSpoil%1").arg(i+1))->isChecked();

		if(type && spoil)
			trayIcon->showMessage("Construction Completed!",
				QString("Say hello to %1!").arg(translateName(type->name)));
		else
			trayIcon->showMessage("Construction Completed!",
				QString("Say hello to your new shipgirl!"));

		updateConstructionsPage();
	} else {
		KCShip *ship = client->ships[dock->shipID];
		KCShipType *type = client->shipTypes[ship->type];

		if(ship) ship->hp.cur = ship->hp.max;

		if(ship && type)
			trayIcon->showMessage("Repair Completed!",
				QString("%1 is all healthy again!").arg(translateName(type->name)));
		else
			trayIcon->showMessage("Repair Completed!",
				QString("Your shipgirl is all healthy again!"));

		updateFleetsPage();
		updateRepairsPage();
	}
}

void KCMainWindow::onMissionCompleted(KCFleet *fleet)
{
	int id = client->fleets.key(fleet);
	trayIcon->showMessage("Expedition Complete",
		QString("Fleet %1 returned from Expedition %2-%3"
			).arg(id).arg(fleet->mission.page).arg(fleet->mission.no));
	updateTimers();
}

void KCMainWindow::on_actionFleets_triggered()
{
	this->showApplication();
	ui->tabBar->setCurrentIndex(0);

	ui->actionFleets->setEnabled(false);
	ui->actionShips->setEnabled(true);
	ui->actionRepairs->setEnabled(true);
	ui->actionConstruction->setEnabled(true);
	ui->stackedWidget->setCurrentWidget(ui->fleetsPage);
}

void KCMainWindow::on_actionShips_triggered()
{
	this->showApplication();
	ui->tabBar->setCurrentIndex(1);

	ui->actionFleets->setEnabled(true);
	ui->actionShips->setEnabled(false);
	ui->actionRepairs->setEnabled(true);
	ui->actionConstruction->setEnabled(true);
	ui->stackedWidget->setCurrentWidget(ui->shipsPage);
}

void KCMainWindow::on_actionRepairs_triggered()
{
	this->showApplication();
	ui->tabBar->setCurrentIndex(2);

	ui->actionFleets->setEnabled(true);
	ui->actionShips->setEnabled(true);
	ui->actionRepairs->setEnabled(false);
	ui->actionConstruction->setEnabled(true);
	ui->stackedWidget->setCurrentWidget(ui->repairsPage);
}

void KCMainWindow::on_actionConstruction_triggered()
{
	this->showApplication();
	ui->tabBar->setCurrentIndex(3);

	ui->actionFleets->setEnabled(true);
	ui->actionShips->setEnabled(true);
	ui->actionRepairs->setEnabled(true);
	ui->actionConstruction->setEnabled(false);
	ui->stackedWidget->setCurrentWidget(ui->constructionPage);
}

void KCMainWindow::on_actionRefresh_triggered()
{
	if(!client->hasCredentials())
	{
		this->askForAPILink();

		// Cancel the refresh if the user pressed Cancel, instead of erroring out
		if(!client->hasCredentials())
			return;
	}

	if(!client->admiral)
		client->requestAdmiral();
	else
		client->requestPort();
	
	client->requestRepairs();
	client->requestConstructions();
}

void KCMainWindow::on_refreshButton_clicked()
{
	on_actionRefresh_triggered();
}

void KCMainWindow::on_actionSettings_triggered()
{
	KCSettingsDialog *settingsDialog = new KCSettingsDialog(this);
	connect(settingsDialog, SIGNAL(apply()), this, SLOT(updateSettingThings()));
	connect(settingsDialog, SIGNAL(finished(int)), settingsDialog, SLOT(deleteLater()));
	settingsDialog->show();
}

void KCMainWindow::on_settingsButton_clicked()
{
	on_actionSettings_triggered();
}

void KCMainWindow::on_tabBar_currentChanged(int index)
{
	if(index == 0)
		on_actionFleets_triggered();
	else if(index == 1)
		on_actionShips_triggered();
	else if(index == 2)
		on_actionRepairs_triggered();
	else
		on_actionConstruction_triggered();
}

void KCMainWindow::on_fleetsTabBar_currentChanged(int index)
{
	//qDebug() << "Fleets page on Tab" << index;
	Q_UNUSED(index);
	updateFleetsPage();
	updateTimers();
}

void KCMainWindow::on_noNetworkSettingsButton_clicked()
{
	this->on_settingsButton_clicked();
}
