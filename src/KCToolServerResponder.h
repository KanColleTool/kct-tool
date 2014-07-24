#ifndef KCTOOLSERVERRESPONDER_H
#define KCTOOLSERVERRESPONDER_H

#include <QObject>
#include <QTcpSocket>
#include <QUrl>
#include <QString>
#include <QHash>
#include <ehttp/HTTPRequestParser.h>

class KCToolServer;
class KCToolServerResponder : public QObject
{
	Q_OBJECT
	
public:
	KCToolServerResponder(QTcpSocket *socket, KCToolServer *parent);
	virtual ~KCToolServerResponder();

	ehttp::HTTPRequestParser parser;
	
protected slots:
	void onSocketReadyRead();

protected:
	QTcpSocket *socket;
	KCToolServer *server;
};

#endif
