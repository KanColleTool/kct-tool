#ifndef KCCLIENT_H
#define KCCLIENT_H

#include <QObject>
#include <QJsonValue>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "KCAdmiral.h"
#include "KCShipType.h"
#include "KCShip.h"
#include "KCFleet.h"
#include "KCDock.h"

class KCToolServer;
class KCClient : public QObject
{
	Q_OBJECT
	friend class KCToolServer;
	friend class KCToolServerResponder;

public:
	typedef std::function<void(KCClient *client, const QVariant&)> processFunc;

	void callPFunc(const QString &path, const QVariant &data);

	typedef enum ErrorCode {
		JsonError = -1,
		Unknown = 0,
		NoError = 1,
		InvalidAPIVersion = 200,
		InvalidCredentials = 201,
		ExpiredAPIToken = 202
	} ErrorCode;

	explicit KCClient(QObject *parent = 0);
	virtual ~KCClient();

	QString server, apiToken;

	KCAdmiral *admiral;
	QMap<int, KCShipType*> shipTypes;
	QMap<int, KCShip*> ships;
	QMap<int, KCFleet*> fleets;
	QMap<int, KCDock*> repairDocks;
	QMap<int, KCDock*> constructionDocks;

	bool hasCredentials();

public slots:
	void setCredentials(QString server, QString apiToken);
	
	void loadMasterData();
	void loadAdmiral();
	void loadPort();
	void loadRepairs();
	void loadConstructions();

signals:
	void focusRequested();

	void credentialsGained();
	void receivedAdmiral();
	void receivedShipTypes();
	void receivedShips();
	void receivedFleets();
	void receivedRepairs();
	void receivedConstructions();
	void requestError(KCClient::ErrorCode error);

	void dockCompleted(KCDock *dock);
	void missionCompleted(KCFleet *fleet);

protected slots:
	void onDockCompleted();
	void onDockShipChanged();
	void onMissionCompleted();

protected:
	void load(QString endpoint, int page = 0);
	QVariant dataFromRawResponse(QString text, ErrorCode *error = 0);

private:
	static const std::map<QString, processFunc> processFuncs;
};

#endif // KCCLIENT_H
