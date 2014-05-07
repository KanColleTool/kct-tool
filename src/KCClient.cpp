#include "KCClient.h"
#include <stdexcept>
#include <QDebug>
#include <QSettings>
#include <QEventLoop>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QFile>
#include "KCShip.h"
#include "KCShipType.h"
#include "KCFleet.h"
#include "KCUtil.h"

#define kClientUseCache 0

void KCClient::callPFunc(const QString &path, const QVariant &data)
{
	try {
		processFunc func = processFuncs.at(path);
		if(func) func(this, data);
	} catch (std::out_of_range e) {
		qDebug() << "Unknown path:" << path;
	}
}

KCClient::KCClient(QObject *parent) :
	QObject(parent),
	admiral(0)
{
	manager = new QNetworkAccessManager(this);

	QSettings settings;
	server = settings.value("server").toString();
	apiToken = settings.value("apiToken").toString();
}

KCClient::~KCClient()
{

}

bool KCClient::hasCredentials()
{
	return (!server.isEmpty() && !server.isEmpty());
}

void KCClient::setCredentials(QString server, QString apiToken)
{
	this->server = server;
	this->apiToken = apiToken;

	if(this->hasCredentials())
	{
		QSettings settings;
		settings.setValue("server", server);
		settings.setValue("apiToken", apiToken);
		settings.sync();

		emit credentialsGained();
	}
}

void KCClient::safeShipTypes() {
	QNetworkRequest request(QString("http://kancolletool.github.io/kctool/mastership.json"));
	QNetworkReply *reply = manager->get(request);
	connect(reply, SIGNAL(finished()), SLOT(onRequestFinished()));
}

void KCClient::requestAdmiral() {
	QNetworkReply *reply = this->call("/api_get_member/basic");
	if(reply) connect(reply, SIGNAL(finished()), SLOT(onRequestFinished()));
}

void KCClient::requestPort() {
	// We need the Admiral's ID for this to work
	if(!admiral)
		return;
	
	QUrlQuery params;
	params.addQueryItem("spi_sort_order", "2");
	params.addQueryItem("api_port", apiPortSignature(admiral->id));
	params.addQueryItem("api_sort_key", "5");
	
	QNetworkReply *reply = this->call("/api_port/port", params);
	if(reply) connect(reply, SIGNAL(finished()), SLOT(onRequestFinished()));
}

void KCClient::requestRepairs() {
	QNetworkReply *reply = this->call("/api_get_member/ndock");
	if(reply) connect(reply, SIGNAL(finished()), SLOT(onRequestFinished()));
}

void KCClient::requestConstructions() {
	QNetworkReply *reply = this->call("/api_get_member/kdock");
	if(reply) connect(reply, SIGNAL(finished()), SLOT(onRequestFinished()));
}

void KCClient::onDockCompleted() {
	emit dockCompleted(qobject_cast<KCDock*>(QObject::sender()));
}

void KCClient::onDockShipChanged() {
	qDebug() << "Construction Started";
}

void KCClient::onMissionCompleted() {
	emit missionCompleted(qobject_cast<KCFleet*>(QObject::sender()));
}

QNetworkReply* KCClient::call(QString endpoint, QUrlQuery params) {
#if kClientUseCache
	QFile file(QString("cache%1.json").arg(endpoint));
	if(file.open(QIODevice::ReadOnly)) {
		qDebug() << "Loading Fixture:" << endpoint;
		QVariant response = this->dataFromRawResponse(file.readAll());

		callPFunc(endpoint, response);

		return 0;
	}
#endif
	QNetworkRequest request(this->urlForEndpoint(endpoint));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setRawHeader("Referer", QString("http://%1/kcs/Core.swf?version=2.0.0").arg(server).toUtf8());
	request.setRawHeader("User-Agent", QString("Mozilla/5.0 (Windows NT 6.1; WOW64; rv:28.0) Gecko/20100101 Firefox/28.0").toUtf8());

	params.addQueryItem("api_verno", "1");
	params.addQueryItem("api_token", apiToken);
	QString query = params.toString(QUrl::FullyEncoded);

	return manager->post(request, query.toUtf8());
}

void KCClient::onRequestFinished() {
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
	if(reply->error() == QNetworkReply::NoError) {
		ErrorCode error;
		QVariant data = this->dataFromRawResponse(reply->readAll(), &error);
		if(data.isValid()) callPFunc(reply->url().path(), data);
		else { qDebug() << reply->request().url() << error; emit requestError(error); }
	} else if(reply->error() == QNetworkReply::UnknownNetworkError) {
		qWarning() << "Connection Failed:" << reply->errorString();
	}
}

QUrl KCClient::urlForEndpoint(QString endpoint) {
	return QUrl(QString("http://%1/kcsapi%2").arg(server, endpoint));
}

QVariant KCClient::dataFromRawResponse(QString text, ErrorCode *error) {
	if(text.startsWith("svdata="))
		text = text.mid(7);

	QJsonParseError jsonErr;
	QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &jsonErr);
	if(jsonErr.error != QJsonParseError::NoError) {
		if(error) *error = JsonError;
		else qWarning() << "JSON Error:" << jsonErr.errorString() << "\n" << text;
		return QVariant();
	}

	QMap<QString, QVariant> data = doc.toVariant().toMap();
	if(data.value("api_result").toInt() != NoError) {
		if(error) *error = (ErrorCode)data.value("api_result").toInt();
		else qWarning() << "API Error:" << jsonErr.errorString() << "\n" << text;
		return QVariant();
	}

	if(error) *error = NoError;
	return data.value("api_data");
}
