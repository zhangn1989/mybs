#include "Server.h"
#include "common.h"

#include <QFile>
#include <QDataStream>
#include <QTcpServer>
#include <QTcpSocket>

Server::Server(QObject *parent)
	: QObject(parent)
{
	m_server = new QTcpServer;
}

Server::~Server()
{
	delete m_server;
}

void Server::startListen(int nPort)
{
	if (m_server->listen(QHostAddress::Any, nPort))
		qDebug() << "listen port "<< nPort << " ok";
	else
		qDebug() << "listen err";
	connect(m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

void Server::onNewConnection()
{
	QTcpSocket *socket = m_server->nextPendingConnection();
	QString ip = socket->peerAddress().toString();
	quint16 port = socket->peerPort();
	qDebug() << "new client connect:" << ip << ":" << port;
	connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
}

void Server::onReadyRead()
{
	qDebug() << "read client message";
	QTcpSocket *socket = dynamic_cast<QTcpSocket *>(sender());
	if (socket)
	{
		QByteArray buff;
		buff = socket->readAll();
		qDebug() << buff;

		QFile file(buff);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return;

		while (!file.atEnd())
		{
			MYBSProtocol *mp = nullptr;
			// 处理中文乱码
			QString line = QString::fromLocal8Bit(file.readLine());
			QString key = line.left(line.indexOf(':'));
			QString value = line.mid(line.indexOf(':') + 1);

			if (key == "text")
			{
				QByteArray ba = value.toLocal8Bit();
				mp = (MYBSProtocol *)malloc(sizeof(mp) + ba.size());
				if (!mp)
					continue;
				memset(mp, 0, sizeof(mp) + value.size());
				mp->type = TEXT;
				mp->length = ba.size();
				memcpy(mp->data, ba.data(), mp->length);
			}
			else if (key == "picture")
			{
				QFile picture(value);
				if (!picture.open(QIODevice::ReadOnly))
					continue;

				QByteArray ba = picture.readAll();
				picture.close();

				mp = (MYBSProtocol *)malloc(sizeof(mp) + ba.size());
				if (!mp)
					continue;
				memset(mp, 0, sizeof(mp) + ba.size());

				mp->type = PICTURE;
				mp->length = ba.count();
				memcpy(mp->data, ba.data(), mp->length);
			}
			else
			{
				continue;
			}

			socket->write((char *)mp, sizeof(mp) + mp->length);
			free(mp);
			mp = nullptr;
		}

		file.close();
		MYBSProtocol mp;
		mp.type = END;
		mp.length = 0;
		socket->write(QByteArray(reinterpret_cast<const char *>(&mp), sizeof(mp)));
	}
}

void Server::onDisconnected()
{
	QTcpSocket *socket = dynamic_cast<QTcpSocket *>(sender());
	if (socket)
	{
		QString ip = socket->peerAddress().toString();
		quint16 port = socket->peerPort();
		qDebug() << "client disconnect:" << ip << ":" << port;
		socket->deleteLater();
	}
}