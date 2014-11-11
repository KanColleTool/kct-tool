#ifndef KCTRANSLATOR_H
#define KCTRANSLATOR_H

#include <LKTranslator.h>

#include <QObject>
#include <QNetworkAccessManager>
#include <QVariant>
#include <QString>

class KCTranslator : public QObject
{
	Q_OBJECT

public:
	static KCTranslator& instance();

public slots:
	QString translate(const QString &line);
	void loadTranslation(QString language = "en");

signals:
	void loadFinished();
	void loadFailed(QString error);

private slots:
	void translationRequestFinished();

protected:
	QNetworkAccessManager manager;
	LKTranslator translator;
	
private:
	// Singleton stuff
	KCTranslator(QObject *parent = 0);
	KCTranslator(const KCTranslator&);
	KCTranslator& operator=(const KCTranslator&);
	virtual ~KCTranslator();
};

#define kcTranslate(_line) (KCTranslator::instance().translate(_line))

#endif
