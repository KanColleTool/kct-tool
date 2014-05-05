#include "KCToolServer.h"
#include <qhttprequest.h>
#include <qhttpresponse.h>
#include <QDebug>

KCToolServer::KCToolServer(QObject *parent):
	QHttpServer(parent)
{
	qDebug() << "Server Created";
	connect(this, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)), this, SLOT(onNewRequest(QHttpRequest*, QHttpResponse*)));
}

KCToolServer::~KCToolServer()
{
	
}

void KCToolServer::onNewRequest(QHttpRequest *req, QHttpResponse *res)
{
	qDebug() << "New Request" << req->url();
	new KCToolServerResponder(req, res, this);
}



// ------------------------------------------------------------------------- //



KCToolServerResponder::KCToolServerResponder(QHttpRequest *req, QHttpResponse *res, KCToolServer *parent):
	QObject(parent), req(req), res(res), server(parent)
{
	// Act once the request is complete
	connect(req, SIGNAL(end()), this, SLOT(onRequestEnd()));
	// Delete this once the reply has been sent
	connect(res, SIGNAL(done()), this, SLOT(deleteLater()));
	
	// Have the request buffer data internally instead of streaming it to us
	req->storeBody();
}

KCToolServerResponder::~KCToolServerResponder()
{
	delete req;
}

void KCToolServerResponder::onRequestEnd()
{
	res->setHeader("Content-Type", "text/plain");
	res->writeHead(200);
	res->write("Lorem ipsum dolor sit amet");
	res->end();
}