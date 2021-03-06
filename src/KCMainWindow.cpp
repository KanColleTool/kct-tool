#include "KCMainWindow.h"
#include "ui_KCMainWindow.h"
#include "KCSettingsDialog.h"
#include "KCTranslator.h"
#include "KCShipType.h"
#include "KCShip.h"
#include "KCDock.h"
#include "KCMacUtils.h"
#include "KCUtil.h"
#include "KCDefaults.h"
#include "KCNotificationCenter.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QShortcut>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

#ifdef Q_OS_WIN
	#include <QtWinExtras>
#endif

KCMainWindow::KCMainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::KCMainWindow),
	trayIcon(0), trayMenu(0), client(0), server(0),
	translation(false),
	lastActivityAt(QDateTime::currentDateTime())
{
	
}

bool KCMainWindow::init()
{
	ui->setupUi(this);

	// Always start at the "No Network" page
	ui->stackedWidget->setCurrentWidget(ui->noNetworkPage);
	ui->topContainer->hide();
	ui->toolBar->hide();

	if(!this->_setupServer()) return false;
	this->_setupClient();
	this->_setupTrayIcon();
	this->_setupUI();
	this->_showDisclaimer();

	// Setup settings and stuff
	connect(&expeditionReminderTimer, SIGNAL(timeout()), this, SLOT(onExpeditionReminderTimeout()));
	expeditionReminderTimer.setSingleShot(true);
	this->updateSettingThings();

	// Load the translation
	KCTranslator &tl = KCTranslator::instance();
	connect(&tl, SIGNAL(loadFinished()), this, SLOT(onTranslationLoadFinished()));
	connect(&tl, SIGNAL(loadFailed(QString)), this, SLOT(onTranslationLoadFailed(QString)));
	tl.loadTranslation();

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
	{
		QString translation = kcTranslate(name);
		if(translation != name)
			return QString("%1 (%2)").arg(translation, name);
	}

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
	connect(client, SIGNAL(receivedAdmiral()), SLOT(onReceivedAdmiral()));
	connect(client, SIGNAL(receivedShipTypes()), SLOT(onReceivedShipTypes()));
	connect(client, SIGNAL(receivedShips()), SLOT(onReceivedShips()));
	connect(client, SIGNAL(receivedFleets()), SLOT(onReceivedFleets()));
	connect(client, SIGNAL(receivedRepairs()), SLOT(onReceivedRepairs()));
	connect(client, SIGNAL(receivedConstructions()), SLOT(onReceivedConstructions()));
	connect(client, SIGNAL(requestError(KCClient::ErrorCode)), SLOT(onRequestError(KCClient::ErrorCode)));
	connect(client, SIGNAL(dockCompleted(KCDock *)), SLOT(onDockCompleted(KCDock *)));
	connect(client, SIGNAL(missionCompleted(KCFleet*)), SLOT(onMissionCompleted(KCFleet*)));

	this->loadData();
}

void KCMainWindow::_setupTrayIcon()
{
	// Create the Tray Icon
	trayIcon = new QSystemTrayIcon(QIcon(":/kancolletool.png"), this);
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
	trayIcon->show();

	// Set up the menu for it, but not if we're on a Mac.
	// On Mac, it's more convenient to have a click bring up the main window
	// (since left-click also brings up the menu there)
#if !defined(__APPLE__)
	trayMenu = new QMenu(tr("KanColleTool"), this);
	trayMenu->addAction(tr("Fleets"), this, SLOT(on_actionFleets_triggered()));
	trayMenu->addAction(tr("Ships"), this, SLOT(on_actionShips_triggered()));
	trayMenu->addAction(tr("Repairs"), this, SLOT(on_actionRepairs_triggered()));
	trayMenu->addAction(tr("Construction"), this, SLOT(on_actionConstruction_triggered()));
	trayMenu->addSeparator();
	trayMenu->addAction(tr("Show"), this, SLOT(showApplication()));
	trayMenu->addAction(tr("Hide"), this, SLOT(hideApplication()));
	trayMenu->addAction(tr("Exit"), qApp, SLOT(quit()));
	trayIcon->setContextMenu(this->trayMenu);
#endif

	KCNotificationCenter::instance().trayIcon = trayIcon;
}

void KCMainWindow::_setupUI()
{
	// Set up Mac-specific styling
#ifdef Q_OS_MAC
	{
		// Right-align Settings on the toolbar
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
	}
#else
	{
		// Only OSX uses the toolbar; hide it everywhere else
		ui->toolBar->hide();
		
		// Add tabs to the tab bar
		ui->tabBar->addTab(tr("Fleets"));
		ui->tabBar->addTab(tr("Ships"));
		ui->tabBar->addTab(tr("Repairs"));
		ui->tabBar->addTab(tr("Construction"));
		
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
		QMessageBox::information(this, tr("Disclaimer"),
#if __APPLE__
			tr(
				"<p>"
				"Disclaimer:"
				"</p>"
			) + 
#endif
			tr(
				"<p>"
				"It's important to note that KanColleTool is not a cheat tool.<br /> "
				"It will not let you do anything the game would not usually let you do."
				"</p>"
				"<p>"
				"Using KanColleTool will not increase your chances of getting banned.<br />"
				"If you're a foreign player, that's already reason enough to ban you, but KCT "
				"is impossible to differentiate from a web browser on their end."
				"</p>"
			)
		);
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
			KCNotificationCenter::instance().notify("closeToTray",
				tr("Still running!"),
				tr("KanColleTool is still running in the tray.\nYou can disable that in the settings.")
			);
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

#ifdef Q_OS_WIN
void KCMainWindow::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	//
	// This deserves some explaining:
	//
	// Due to what is possibly a Qt bug (I'm not sure), translucent windows on
	// Windows 7+ won't clear their backgrounds properly, and thus anything that
	// changes the shape of the translucent area will leave ghost images behind.
	//
	// This manually creates a QPainter, sets the composition mode to "ignore the
	// color I'm painting with and clear instead" and paints a black rectangle over
	// the whole window, thus forcing it all to go fully transparent.
	//
	// Without this, Windows' style of highlighting a tab by enlarging it will bug
	// out, and the larger frame will stay filled when another tab is activated.
	//

	QPainter painter(this);
	painter.setCompositionMode(QPainter::CompositionMode_Clear);
	painter.fillRect(0, 0, this->width(), this->height(), QColor(0,0,0));
}
#endif

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
				hpBar->setToolTip(tr("Repair Time: %1, Repair Cost: %2 Steel, %3 Fuel").arg(ship->repairTime.toString("H:mm:ss"), QString::number(ship->repairCost.steel), QString::number(ship->repairCost.fuel)));
			else
				hpBar->setToolTip(tr("Healthy"));

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
				box->setTitle(tr("(Loading...)"));
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
			TABLE_SET_ITEM(ui->shipsTable, row, 8, tr("(Loading...)"));
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
			nameLabel->setText(tr("(Locked)"));
			repairTimerLabel->setText("");
		}
		else if(dock->state == KCDock::Empty)
		{
			box->setEnabled(true);
			nameLabel->setText(tr("(Empty)"));
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
					nameLabel->setText(tr("(Loading...)"));
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
			nameLabel->setText(tr("(Locked)"));
			buildTimerLabel->setText("");
			spoilCheckbox->hide();
		}
		else if(dock->state == KCDock::Empty)
		{
			box->setEnabled(true);
			nameLabel->setText(tr("(Empty)"));
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
				nameLabel->setText(ship ? translateName(ship->name) : tr("(Loading...)"));
			}
			else
				nameLabel->setText(tr("???"));

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
			QString status = tr("Combat-Ready");
			QTime dT;

			// Check if the fleet is out on an expedition
			if(fleet->mission.page > 0 && fleet->mission.no > 0 && fleet->mission.complete > QDateTime::currentDateTime())
			{
				busy = true;
				status = tr("Doing Expedition %1-%2").arg(fleet->mission.page).arg(fleet->mission.no);
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
						status = tr("%1 is taking a bath").arg(type ? translateName(type->name) : tr("(Loading...)"));
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

	// Notification flags
	KCNotificationCenter &nc = KCNotificationCenter::instance();

	notifyExpeditionReminder = settings.value("notifyExpeditionReminder", kDefaultNotifyExpeditionReminder).toBool();
	notifyExpeditionReminderInterval = settings.value("notifyExpeditionReminderInterval", kDefaultNotifyExpeditionReminderInterval).toInt();
	notifyExpeditionReminderRepeat = settings.value("notifyExpeditionReminderRepeat", kDefaultNotifyExpeditionRepeat).toBool();
	notifyExpeditionReminderRepeatInterval = settings.value("notifyExpeditionReminderRepeatInterval", kDefaultNotifyExpeditionRepeatInterval).toInt();
	notifyExpeditionReminderSuspend = settings.value("notifyExpeditionReminderSuspend", kDefaultNotifyExpeditionSuspend).toBool();
	notifyExpeditionReminderSuspendInterval = settings.value("notifyExpeditionReminderSuspendInterval", kDefaultNotifyExpeditionSuspendInterval).toInt();

	nc.enabled = settings.value("notify", kDefaultNotify).toBool();
	nc.enabledNotifications["repairComplete"] = settings.value("notifyRepairs", kDefaultNotifyRepairs).toBool();
	nc.enabledNotifications["constructionComplete"] = settings.value("notifyConstruction", kDefaultNotifyConstruction).toBool();
	nc.enabledNotifications["expeditionComplete"] = settings.value("notifyExpedition", kDefaultNotifyExpedition).toBool();
	nc.enabledNotifications["expeditionReminder"] = notifyExpeditionReminder;

	nc.backend = (KCNotificationCenter::Backend)settings.value("notificationBackend", kDefaultNotificationBackend).toInt();

	this->checkExpeditionStatus();
}

void KCMainWindow::loadData()
{
	client->loadMasterData();
	client->loadAdmiral();
	client->loadPort();
	client->loadRepairs();
	client->loadConstructions();
}

void KCMainWindow::leaveNoNetworkPage()
{
	if(ui->stackedWidget->currentWidget() == ui->noNetworkPage)
	{
		this->setUpdatesEnabled(false);
#ifdef Q_OS_MAC
		ui->toolBar->show();
#else
		ui->topContainer->show();
#endif
		this->on_actionFleets_triggered();
		this->setUpdatesEnabled(true);
	}
}

void KCMainWindow::onTranslationLoadFinished()
{
	qDebug() << "Received Translation Data!";
	// Update all the things!
	this->updateFleetsPage();
	this->updateShipsPage();
	this->updateRepairsPage();
	this->updateConstructionsPage();
}

void KCMainWindow::onTranslationLoadFailed(QString error)
{
	qDebug() << "Failed to load Translation Data..." << error;
	QMessageBox::StandardButton button = QMessageBox::warning(this,
		tr("Couldn't load Translation"),
		tr("You may choose to continue without translation data, but everything will be in Japanese."),
		QMessageBox::Ok|QMessageBox::Retry, QMessageBox::Retry
	);
	if(button == QMessageBox::Retry)
		KCTranslator::instance().loadTranslation();
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
	lastActivityAt = QDateTime::currentDateTime();
}

void KCMainWindow::onReceivedFleets()
{
	qDebug() << "Received Player Fleet Data" << client->fleets.size();

	// If we don't have enough tabs, add some more
	for(int i = ui->fleetsTabBar->count(); i < client->fleets.size(); i++)
		ui->fleetsTabBar->addTab(tr("Fleet %1").arg(i + 1));

	// If we're on an active tab, update it
	if(ui->fleetsTabBar->currentIndex() < client->fleets.size())
	{
		updateFleetsPage();
		updateTimers();
	}

	// Start expedition reminder timers if applicable
	checkExpeditionStatus();

	leaveNoNetworkPage();
	lastActivityAt = QDateTime::currentDateTime();
}

void KCMainWindow::onReceivedRepairs()
{
	qDebug() << "Received Player Repairs Data" << client->repairDocks.size();
	updateRepairsPage();
	leaveNoNetworkPage();
	lastActivityAt = QDateTime::currentDateTime();
}

void KCMainWindow::onReceivedConstructions()
{
	qDebug() << "Received Player Constructions Data" << client->constructionDocks.size();
	updateConstructionsPage();
	leaveNoNetworkPage();
	lastActivityAt = QDateTime::currentDateTime();
}

void KCMainWindow::onRequestError(KCClient::ErrorCode error)
{
	switch(error)
	{
		case KCClient::JsonError:
			QMessageBox::warning(this,
				tr("JSON Error"),
				tr("The response was malformed JSON and could not be parsed. This could mean that there's something messing with your internet connection."));
			break;
		case KCClient::InvalidAPIVersion:
			QMessageBox::critical(this,
				tr("Invalid API Version"),
				tr("KanColle changed their API, and this program is outdated."));
			qApp->quit();
			break;	// OCD
		case KCClient::InvalidCredentials:
			QMessageBox::warning(this,
				tr("Expired API Token"),
				tr("Your API Token has expired, and you need a new one."));
			break;
		default:
			QMessageBox::warning(this,
				tr("Unknown Error"),
				tr("An unknown error occurred."));
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

		KCNotificationCenter::instance().notify("constructionComplete",
			tr("Construction Completed!"),
			tr("Say hello to %1!").arg((type && spoil) ? translateName(type->name) : tr("your new shipgirl"))
		);

		updateConstructionsPage();
	} else {
		KCShip *ship = client->ships[dock->shipID];
		KCShipType *type = client->shipTypes[ship->type];

		if(ship) ship->hp.cur = ship->hp.max;

		KCNotificationCenter::instance().notify("repairComplete",
			tr("Repair Completed!"),
			tr("%1 is all healthy again!").arg((ship && type) ? translateName(type->name) : tr("Your shipgirl"))
		);

		updateFleetsPage();
		updateRepairsPage();
	}

	lastActivityAt = QDateTime::currentDateTime();
}

void KCMainWindow::onMissionCompleted(KCFleet *fleet)
{
	int id = client->fleets.key(fleet);
	KCNotificationCenter::instance().notify("expeditionComplete",
		tr("Expedition Complete"),
		tr("Fleet %1 returned from Expedition %2-%3").arg(id).arg(fleet->mission.page).arg(fleet->mission.no)
	);
	updateTimers();

	lastActivityAt = QDateTime::currentDateTime();
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

void KCMainWindow::on_noNetworkSettingsButton_clicked()
{
	this->on_settingsButton_clicked();
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

void KCMainWindow::checkExpeditionStatus()
{
	if(notifyExpeditionReminder)
	{
		bool youShouldPutOutExpeditions = true;
		foreach(KCFleet *fleet, client->fleets)
		{
			if(!fleet) continue;	// There will be NULLs in there
			if(fleet->mission.complete > QDateTime::currentDateTime())
				youShouldPutOutExpeditions = false;
		}

		if(youShouldPutOutExpeditions)
		{
			qDebug() << "Starting Expedition Reminder Timer";
			expeditionReminderTimer.start(notifyExpeditionReminderInterval * 1000);
		}
		else
		{
			qDebug() << "Stopping Expedition Reminder Timer";
			expeditionReminderTimer.stop();
		}
	}
	else
		expeditionReminderTimer.stop();
}

void KCMainWindow::onExpeditionReminderTimeout()
{
	KCNotificationCenter::instance().notify("expeditionReminder",
		tr("Remember your expeditions!"),
		tr("You currently don't have any expeditions out.\nYou can disable these messages in the settings.")
	);

	if(notifyExpeditionReminder && notifyExpeditionReminderRepeat &&
			(!notifyExpeditionReminderSuspend || lastActivityAt.secsTo(QDateTime::currentDateTime()) < notifyExpeditionReminderSuspendInterval))
		expeditionReminderTimer.start(notifyExpeditionReminderRepeatInterval * 1000);
}
