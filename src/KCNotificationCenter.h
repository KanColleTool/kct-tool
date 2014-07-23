#ifndef KCNOTIFICATIONCENTER_H
#define KCNOTIFICATIONCENTER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QSystemTrayIcon>
#include <growl.h>

/**
 * Singleton for abstracting away the user's notification system, eg. Growl,
 * Notification Center, libnotify, etc. outside of what QSystemTrayIcon can do.
 */
class KCNotificationCenter : public QObject
{
public:
	/// Returns the singleton instance
	static KCNotificationCenter& instance();



	/// Defines the type of backend to be used
	enum Backend
	{
		DefaultBackend,	///< Qt's default system for the platform
		GrowlBackend		///< Explicitly use Growl
	};



	/// Are notifications enabled at all?
	bool enabled;

	/// Which notification backend should we use?
	Backend backend;

	/**
	 * Notification IDs that are explicitly enabled or disabled; update this from
	 * the user's preferences.
	 * @see notify()
	 */
	QMap<QString, bool> enabledNotifications;



	/// The tray icon to use for displaying Qt's native notifications
	QSystemTrayIcon *trayIcon;

public slots:
	/**
	 * Displays a notification to the user.
	 * 
	 * IDs will be checked against the enabledNotifications map, defaulting to the
	 * value of defaultToEnabled, so that you can easily disable notifications
	 * based on user preferences.
	 * 
	 * @param id An ID for the notification; not shown to the user
	 * @param title The title of the notification bubble
	 * @param message The message to display
	 * @param defaultToEnabled Whether the notification should default to being
	 *        enabled if not specified in the settings
	 */
	void notify(const QString &id, const QString &title, const QString &message, bool defaultToEnabled = true);
	
private:
	// No manual construction of singletons!
	KCNotificationCenter(QObject *parent = 0);
	KCNotificationCenter(const KCNotificationCenter&);
	virtual ~KCNotificationCenter();
};

#endif
