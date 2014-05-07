#ifndef KCTOOLSERVERRESPONDER_H
#define KCTOOLSERVERRESPONDER_H

#include <QObject>
#include <QTcpSocket>
#include <QUrl>
#include <QString>
#include <QHash>

#include <http_parser.h>

class KCToolServer;
class KCToolServerResponder : public QObject
{
	Q_OBJECT
	
public:
	KCToolServerResponder(QTcpSocket *socket, KCToolServer *parent);
	virtual ~KCToolServerResponder();
	
	http_method method;
	QUrl url;
	QHash<QString,QString> headers;
	QByteArray body;
	
	QString currentUrlString, currentHeaderField, currentHeaderValue;
	bool dataComplete;
	
protected slots:
	void onSocketReadyRead();

protected:
	void respond(QTcpSocket *socket);
	void writeStatusLine(int status, QString reason);
	void writeHeader(QString key, QString value);
	void writeBody(QByteArray body = QByteArray(), QString contentType = "");
	
	QTcpSocket *socket;
	KCToolServer *server;
	
	http_parser parser;
	http_parser_settings settings;
};

#endif
