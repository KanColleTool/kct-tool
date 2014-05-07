#include "KCToolServerResponder.h"
#include "KCToolServer.h"
#include "KCClient.h"
#include <QDebug>

KCToolServerResponder::KCToolServerResponder(QTcpSocket *socket, KCToolServer *parent):
	QObject(parent),
	dataComplete(false),
	socket(socket), server(parent),
	parser(), settings()
{
	http_parser_init(&parser, HTTP_REQUEST);
	parser.data = this;
	
	// settings.on_message_begin wouldn't do anything anyways
	// settings.on_status is never called for requests
	settings.on_url = [](http_parser *parser, const char *data, size_t size) -> int
	{
		KCToolServerResponder *responder = static_cast<KCToolServerResponder*>(parser->data);
		// Note: this isn't turned into a QUrl until on_headers_complete, because really,
		// we don't need it until after that anyways, and doing it in on_header_field would
		// look ugly (and be a waste of (microscopic amounts of) processing power)
		responder->currentUrlString += QString::fromUtf8(data, size);
		
		return 0;
	};
	settings.on_header_field = [](http_parser *parser, const char *data, size_t size) -> int
	{
		KCToolServerResponder *responder = static_cast<KCToolServerResponder*>(parser->data);
		// If we were parsing another header, put that into the pile and clear
		if(responder->currentHeaderValue.length() > 0)
		{
			responder->headers.insert(responder->currentHeaderField.toLower(), responder->currentHeaderValue.toLower());
			responder->currentHeaderField.clear();
			responder->currentHeaderValue.clear();
		}
		responder->currentHeaderField += QString::fromUtf8(data, size);
		
		return 0;
	};
	settings.on_header_value = [](http_parser *parser,  const char *data, size_t size) -> int
	{
		KCToolServerResponder *responder = static_cast<KCToolServerResponder*>(parser->data);
		responder->currentHeaderValue += QString::fromUtf8(data, size);
		
		return 0;
	};
	settings.on_headers_complete = [](http_parser *parser) -> int
	{
		KCToolServerResponder *responder = static_cast<KCToolServerResponder*>(parser->data);
		responder->method = (http_method)parser->method;
		responder->url = responder->currentUrlString;
		responder->headers.insert(responder->currentHeaderField.toLower(), responder->currentHeaderValue.toLower());
		
		return 0;
	};
	settings.on_body = [](http_parser *parser, const char *data, size_t size) -> int
	{
		KCToolServerResponder *responder = static_cast<KCToolServerResponder*>(parser->data);
		responder->body += QByteArray::fromRawData(data, size);
		
		return 0;
	};
	settings.on_message_complete = [](http_parser *parser) -> int
	{
		KCToolServerResponder *responder = static_cast<KCToolServerResponder*>(parser->data);
		responder->dataComplete = true;
		
		return 0;
	};
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}

KCToolServerResponder::~KCToolServerResponder()
{
	
}

void KCToolServerResponder::onSocketReadyRead()
{
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(QObject::sender());
	QByteArray data = socket->readAll();
	
	int parsed = http_parser_execute(&parser, &settings, data.data(), data.length());
	if(parsed != data.length())
		socket->close();
	else if(dataComplete)
		this->respond(socket);
}

void KCToolServerResponder::respond(QTcpSocket *socket)
{
	// Call a handler function; for now, only POSTs are supported
	if(method == HTTP_POST)
	{
		QVariant data = server->client->dataFromRawResponse(body);
		server->client->callPFunc(url.path(), body);
	}
	
	// Always respond with a HTTP 204 No Content for now
	// This might change later, who knows
	this->writeStatusLine(204, "No Content");
	this->writeBody();
	
	// Close the connection if requested to do so
	// Note: headers are lowercased
	if(headers.value("connection") == "close")
		socket->disconnectFromHost();
}

void KCToolServerResponder::writeStatusLine(int status, QString reason)
{
	socket->write(QString("HTTP/1.1 %1 %2\r\n").arg(status).arg(reason).toLatin1());
}

void KCToolServerResponder::writeHeader(QString key, QString value)
{
	socket->write(QString("%1: %2\r\n").arg(key).arg(value).toLatin1());
}

void KCToolServerResponder::writeBody(QByteArray data, QString contentType)
{
	if(!contentType.isEmpty()) this->writeHeader("Content-Type", contentType);
	if(!data.isEmpty()) this->writeHeader("Content-Length", QByteArray::number(data.length()));
	
	socket->write("\r\n");
	
	if(!data.isEmpty()) socket->write(data);
}
