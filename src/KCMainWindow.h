#ifndef KCMAINWINDOW_H
#define KCMAINWINDOW_H

#include "KCClient.h"
#include "KCToolServer.h"

#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QPointer>
#include <QTimer>
#include <QNetworkAccessManager>

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

protected slots:
	void postConstructorSetup();

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

	// Workaround for a Qt/Windows bug
#ifdef Q_OS_WIN
	virtual void paintEvent(QPaintEvent *event);
#endif

public:
	bool isApplicationActive();

public slots:
	void toggleApplication();
	void showApplication();
	void hideApplication();

	void updateFleetsPage();
	void updateShipsPage();
	void updateRepairsPage();
	void updateConstructionsPage();
	void updateTimers();
	void updateSettingThings();
	void loadData();
	void leaveNoNetworkPage();

private slots:
	void onTranslationLoadFinished();
	void onTranslationLoadFailed(QString error);
	void onReceivedAdmiral();
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
	void on_actionSettings_triggered();
	void on_settingsButton_clicked();
	void on_noNetworkSettingsButton_clicked();

	void on_tabBar_currentChanged(int index);
	void on_fleetsTabBar_currentChanged(int index);

	void checkExpeditionStatus();
	void onExpeditionReminderTimeout();

private:
	Ui::KCMainWindow *ui;

	QSystemTrayIcon *trayIcon;
	QMenu *trayMenu;
	QTimer timerUpdateTimer;

	KCClient *client;
	KCToolServer *server;
	QNetworkAccessManager manager;

	bool translation;
	//bool notify, notifyRepairs, notifyConstruction, notifyExpedition;
	bool notifyExpeditionReminder, notifyExpeditionReminderRepeat, notifyExpeditionReminderSuspend;
	int notifyExpeditionReminderInterval, notifyExpeditionReminderRepeatInterval, notifyExpeditionReminderSuspendInterval;

	QTimer expeditionReminderTimer;
	QDateTime lastActivityAt;
};

#endif // KCMAINWINDOW_H
