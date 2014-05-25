#include "KCClient.h"
#include <stdexcept>
#include <QSettings>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
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
	
}

KCClient::~KCClient()
{

}

void KCClient::loadMasterData() {
	this->load("/kcsapi/api_start2");
}

void KCClient::loadAdmiral() {
	this->load("/kcsapi/api_get_member/basic");
}

void KCClient::loadPort() {
	this->load("/kcsapi/api_port/port");
}

void KCClient::loadRepairs() {
	this->load("/kcsapi/api_get_member/ndock");
}

void KCClient::loadConstructions() {
	this->load("/kcsapi/api_get_member/kdock");
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

void KCClient::load(QString endpoint, int page) {
	QString cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
	
	QString path(cacheDir + "/userdata" + endpoint);
	if(page != 0) path += "__" + QString::number(page);
	path += ".json";
	
	QFile file(path);
	if(file.open(QIODevice::ReadOnly)) {
		ErrorCode error;
		QVariant data = this->dataFromRawResponse(file.readAll(), &error);
		if(data.isValid()) {
			callPFunc(endpoint, data);
		} else {
			qDebug() << "Error loading" << endpoint << ":" << error;
			emit requestError(error);
		}
	} else {
		qDebug() << "Couldn't open" << endpoint << "(" << path << "):" << file.error();
	}
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
