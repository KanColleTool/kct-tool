#ifndef KCMAINWINDOW_H
#define KCMAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QPointer>
#include <QTimer>
#include <QNetworkAccessManager>
#include "KCClient.h"
#include "KCToolServer.h"

namespace Ui {
	class KCMainWindow;
}

class KCMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit KCMainWindow(QWidget *parent = 0);
	bool init();
	~KCMainWindow();

private:
	QString translateName(const QString &name);

	bool _setupServer();
	void _setupClient();
	void _setupTrayIcon();
	void _setupUI();
	void _showDisclaimer();

protected:
	virtual void closeEvent(QCloseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

public:
	bool isApplicationActive();

public slots:
	void toggleApplication();
	void showApplication();
	void hideApplication();

	void askForAPILink();
	void updateFleetsPage();
	void updateShipsPage();
	void updateRepairsPage();
	void updateConstructionsPage();
	void updateTimers();
	void updateSettingThings();
	void leaveNoNetworkPage();

private slots:
	void onTranslationLoadFinished();
	void onTranslationLoadFailed(QString error);
	void onReceivedShipTypes();
	void onReceivedShips();
	void onReceivedFleets();
	void onReceivedRepairs();
	void onReceivedConstructions();
	void onRequestError(KCClient::ErrorCode error);

	void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
	void onDockCompleted(KCDock *dock);
	void onMissionCompleted(KCFleet *fleet);

	void on_actionFleets_triggered();
	void on_actionShips_triggered();
	void on_actionRepairs_triggered();
	void on_actionConstruction_triggered();
	void on_actionRefresh_triggered();
	void on_actionSettings_triggered();

	void on_fleetsTabBar_currentChanged(int index);
	void on_noNetworkSettingsButton_clicked();

private:
	Ui::KCMainWindow *ui;

	QSystemTrayIcon *trayIcon;
	QMenu *trayMenu;
	QTimer timerUpdateTimer, refreshTimer;

	KCClient *client;
	KCToolServer *server;
	QNetworkAccessManager manager;

	bool apiLinkDialogOpen;
	bool useNetwork;
	bool translation;
};

#endif // KCMAINWINDOW_H