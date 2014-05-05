#ifndef KCTOOLSERVER_H
#define KCTOOLSERVER_H

#include <qhttpserver.h>
#include "KCClient.h"

class KCToolServer : public QHttpServer
{
	Q_OBJECT
	
	friend class KCToolServerResponder;
	
public:
	KCToolServer(QObject *parent = 0);
	virtual ~KCToolServer();
	
	bool enabled;
	
	inline void setClient(KCClient *client) { this->client = client; }
	
protected slots:
	void onNewRequest(QHttpRequest *req, QHttpResponse *res);
	
protected:
	KCClient *client;
};



class KCToolServerResponder : QObject
{
	Q_OBJECT
	
public:
	KCToolServerResponder(QHttpRequest *req, QHttpResponse *res, KCToolServer *parent);
	virtual ~KCToolServerResponder();
	
protected slots:
	void onRequestEnd();
	
protected:
	QHttpRequest *req;
	QHttpResponse *res;
	KCToolServer *server;
};

#endif
