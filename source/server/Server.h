#pragma once

#include <QObject>

class QTcpServer;
class Server : public QObject
{
	Q_OBJECT

public:
	explicit Server(QObject *parent = nullptr);
	~Server();

	void startListen(int nPort);
private:
	QTcpServer *m_server;

public slots :
	void onNewConnection();
	void onReadyRead();
	void onDisconnected();
};
