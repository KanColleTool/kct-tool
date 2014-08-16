#ifndef KCGAMEOBJECT_H
#define KCGAMEOBJECT_H

#include <QObject>
#include <QVariantMap>
#include <QDateTime>

// QSequentialIterable isn't available before Qt 5.2, and Ubuntu is stuck on 5.0! (╯°□°)╯︵ ┻━┻
// So just because they can't keep up-to-date with things, I'll just do the quick-and-dirty
// workaround of defining QSequentialIterable to be a QList. No one will ever be able to tell.
// (Except, you know, if they look at the performance, which will be one hell of a lot worse...)
#if QT_VERSION >= 0x050200
	#include <QSequentialIterable>
#else
	#include <QList>
	#define QSequentialIterable QList<QVariant>
#endif

class KCGameObject : public QObject {
	Q_OBJECT

public:
	KCGameObject(QObject *parent = 0) : QObject(parent) {}
	virtual ~KCGameObject() {}

	virtual void loadFrom(const QVariantMap &data, int loadId = 0)=0;

	// Extract a single item from a variant map into a T&
	template<typename T>
	static void extract(const QVariantMap &source, T& dest, const QString& key) {
		const QVariant &prop = source[key];
		if(!prop.isNull())
			dest = source[key].value<T>();
		else dest = T();
	}

	// Extract a list of items from a variant map into a T[]
	template<typename T>
	static void extract(const QVariantMap &source, T dest[], int count, const QString& key) {
		const QVariant &prop = source[key];
		if(!prop.isNull())
		{
			int i = 0;
			foreach(const QVariant &v, prop.value<QSequentialIterable>())
			{
				dest[i] = v.value<T>();
				if(++i >= count) break;
			}
		}
		else
			for(int i = 0; i < count; i++)
				dest[i] = T();
	}

	// Extract a single item from a list into a T&
	template<typename T>
	static void extract(const QVariantMap &source, T& dest, const QString &key, int index) {
		const QVariant &prop = source[key];
		if(!prop.isNull())
			dest = prop.value<QSequentialIterable>().at(index).value<T>();
		else dest = T();
	}

	// Extract just the count of a list into an int&
	static void extractCount(const QVariantMap &source, int& dest, const QString &key) {
		const QVariant &prop = source[key];
		if(!prop.isNull())
			dest = prop.value<QSequentialIterable>().size();
		else dest = 0;
	}

	// Extract a timestamp given as msecs since start of epoch into a QDateTime
	static void extractTimestamp(const QVariantMap &source, QDateTime& dest, const QString &key) {
		qint64 tmp;
		extract(source, tmp, key);
		dest = QDateTime::fromMSecsSinceEpoch(tmp);
	}
};

#endif // KCGAMEOBJECT_H
