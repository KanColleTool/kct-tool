#include "KCNotificationCenter.h"
#include <QDebug>

KCNotificationCenter& KCNotificationCenter::instance()
{
	static KCNotificationCenter _instance;
	return _instance;
}

KCNotificationCenter::KCNotificationCenter(QObject *parent):
	backend(DefaultBackend),
	trayIcon(0)
{

}

KCNotificationCenter::~KCNotificationCenter()
{

}

void KCNotificationCenter::notify(const QString &id, const QString &title, const QString &message, bool defaultToEnabled)
{
	if(!enabledNotifications.value(id, defaultToEnabled))
		return;

	if(backend == GrowlBackend)
	{
		// We need to keep these byte arrays around, or constData() would yield a
		// dangling pointer, as the byte arrays would be but fleeting temporaries
		QByteArray utf8_id = id.toUtf8();
		QByteArray utf8_title = title.toUtf8();
		QByteArray utf8_message = message.toUtf8();
		
		growl("localhost", "KanColleTool", utf8_id.constData(), utf8_title.constData(), utf8_message.constData(), "http://i.imgur.com/LeHyDub.png", NULL, NULL);
	}
	else
	{
		trayIcon->showMessage(title, message);
	}
}
