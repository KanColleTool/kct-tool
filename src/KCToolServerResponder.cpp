#include "KCToolServerResponder.h"
#include "KCToolServer.h"

KCToolServerResponder::KCToolServerResponder(QTcpSocket *socket, KCToolServer *parent):
	QObject(parent),
	socket(socket), server(parent),
	parser(0), settings(0)
{
	parser = (http_parser*)malloc(sizeof(http_parser));
	http_parser_init(parser, HTTP_REQUEST);
	parser->data = this;
}

KCToolServerResponder::~KCToolServerResponder()
{
	
}
