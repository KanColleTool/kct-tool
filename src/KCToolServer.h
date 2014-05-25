#ifndef KCTOOLSERVER_H
#define KCTOOLSERVER_H

#include <QTcpServer>

class QTcpSocket;
class KCClient;
class KCToolServer : public QTcpServer
{
	Q_OBJECT
	friend class KCToolServerResponder;

public:
	KCToolServer(QObject *parent = 0);
	virtual ~KCToolServer();

	inline void setClient(KCClient *c) { this->client = c; }

protected slots:
	void onNewConnection();

protected:
	KCClient *client;
};

#endif
