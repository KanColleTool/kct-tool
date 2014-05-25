#include "KCToolServer.h"
#include <QTcpSocket>
#include "KCClient.h"
#include "KCToolServerResponder.h"

KCToolServer::KCToolServer(QObject *parent) :
	QTcpServer(parent), client(0)
{
	connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

KCToolServer::~KCToolServer()
{
	
}

void KCToolServer::onNewConnection()
{
	while(this->hasPendingConnections())
	{
		QTcpSocket *socket = this->nextPendingConnection();
		new KCToolServerResponder(socket, this);
	}
}
