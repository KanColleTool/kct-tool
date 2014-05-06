#ifndef KCTOOLSERVERRESPONDER_H
#define KCTOOLSERVERRESPONDER_H

#include <QObject>
#include <QTcpSocket>

#include <http_parser.h>

class KCToolServer;
class KCToolServerResponder : public QObject
{
	Q_OBJECT
	
public:
	KCToolServerResponder(QTcpSocket *socket, KCToolServer *parent);
	virtual ~KCToolServerResponder();

protected:
	QTcpSocket *socket;
	KCToolServer *server;
	
	http_parser *parser;
	http_parser_settings *settings;
};

#endif
