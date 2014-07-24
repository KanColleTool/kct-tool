#include "KCToolServerResponder.h"
#include "KCToolServer.h"
#include "KCClient.h"

#include <QDebug>
#include <ehttp/URL.h>
#include <ehttp/HTTPResponse.h>

using namespace ehttp;

KCToolServerResponder::KCToolServerResponder(QTcpSocket *socket, KCToolServer *parent):
	QObject(parent),
	socket(socket), server(parent)
{
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

	HTTPRequestParser::Status status = parser.parseChunk(data.constData(), data.size());
	if(status == HTTPRequestParser::GotRequest)
	{
		std::shared_ptr<HTTPRequest> req = parser.req();

		QByteArray body = QByteArray::fromRawData(req->body.data(), req->body.size());
		QVariant data = server->client->dataFromRawResponse(body);
		server->client->callPFunc(QString::fromStdString(ehttp::URL(req->url).path), data);

		std::shared_ptr<HTTPResponse> res = std::make_shared<HTTPResponse>(req);
		res->onData = [=](std::shared_ptr<HTTPResponse> res, std::vector<char> data) {
			qDebug() << "onData: " << QByteArray::fromRawData(data.data(), data.size());
			socket->write(data.data(), data.size());
		};
		res->onEnd = [=](std::shared_ptr<HTTPResponse> res) {
			qDebug() << "onEnd";
			if(req->headers.at("connection") == "close")
				socket->close();
		};

		// To return some actual data, add some header() and write() calls to this
		try
		{
			res->begin(204)
				->end();
		}
		catch(std::exception &e)
		{
			qWarning() << e.what();
		}
	}
	else if(status == HTTPRequestParser::Error)
		socket->close();
}
