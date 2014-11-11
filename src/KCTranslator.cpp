#include "KCTranslator.h"
#include "KCUtil.h"
#include "KCDefaults.h"
#include <LKUtil.h>

#include <QUrl>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

KCTranslator& KCTranslator::instance()
{
	static KCTranslator _instance;
	return _instance;
}

KCTranslator::KCTranslator(QObject *parent):
	QObject(parent)
{
	
}

KCTranslator::~KCTranslator()
{
	
}

QString KCTranslator::translate(const QString &line)
{
	return QString::fromStdString(translator.translate(line.toStdString()));
}

void KCTranslator::loadTranslation(QString language)
{
	translator.loadStatus = LKTranslator::LoadStatusLoading;
	
	QNetworkReply *reply = manager.get(QNetworkRequest(QString("http://api.comeonandsl.am/translation/%1/").arg(language)));
	connect(reply, SIGNAL(finished()), this, SLOT(translationRequestFinished()));
}

void KCTranslator::translationRequestFinished()
{
	// Read the response body
	QNetworkReply *reply(qobject_cast<QNetworkReply*>(QObject::sender()));
	if(reply->error() != QNetworkReply::NoError)
	{
		translator.loadStatus = LKTranslator::LoadStatusError;
		emit loadFailed(QString("Network Error: %1").arg(reply->errorString()));
		return;
	}
	QByteArray body(reply->readAll());
	
	// Parse the JSON
	QJsonParseError error;
	QJsonDocument doc(QJsonDocument::fromJson(body, &error));
	if(error.error != QJsonParseError::NoError)
	{
		translator.loadStatus = LKTranslator::LoadStatusError;
		emit loadFailed(QString("JSON Error: %1").arg(error.errorString()));
		return;
	}
	QJsonObject root(doc.object());
	
	// Check the response
	int success = (int) root.value("success").toDouble();
	if(success != 1)
	{
		translator.loadStatus = LKTranslator::LoadStatusError;
		emit loadFailed(QString("API Error %1").arg(success));
		return;
	}
	
	// Parse the translation data
	QVariantMap translation = root.value("translation").toObject().toVariantMap();
	translator.loadStatus = LKTranslator::LoadStatusLoaded;
	translator.translationData.clear();
	for(auto it = translation.begin(); it != translation.end(); it++)
		translator.translationData[crc32(it.key().toStdString())] = it.value().toString().toStdString();
	
	emit loadFinished();
}
