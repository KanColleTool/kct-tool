#include "KCNotificationCenter.h"
#include <QDebug>

KCNotificationCenter& KCNotificationCenter::instance()
{
	static KCNotificationCenter _instance;
	return _instance;
}


KCNotificationCenter::KCNotificationCenter(QObject *parent):
	backend(QtDefault),
	trayIcon(0)
{

}

void KCNotificationCenter::notify(const QString &id, const QString &title, const QString &message, bool defaultToEnabled)
{
	if(!enabledNotifications.value(id, defaultToEnabled))
		return;

	switch(backend)
	{
	case Growl:
		qWarning() << "Grown notifications are not yet implemented!";
	default:
		trayIcon->showMessage(title, message);
	}
}
