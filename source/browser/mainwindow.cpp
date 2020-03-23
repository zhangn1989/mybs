#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.h"

#include <malloc.h>

#include <QTcpSocket>
#include <QHostAddress>
#include <QPushButton>
#include <QTextCodec>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	m_cursor = 0;
	m_surplus = 0;
	m_tempBuff = nullptr;
	connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onPushButtonClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
	if (m_tempBuff)
		delete m_tempBuff;
}

void MainWindow::onPushButtonClicked(bool checked)
{
	QString url = ui->lineEdit->text();
	QString ip = url.left(url.indexOf(':'));
	QString port = url.mid(url.indexOf(':') + 1, url.indexOf('/') - url.indexOf(':') - 1);
	QString file = url.mid(url.indexOf('/') + 1);

	QTcpSocket *socket = new QTcpSocket;
	connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
	connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
	socket->connectToHost(ip, port.toUInt());
	if (socket->waitForConnected(-1))
	{
		socket->write(file.toUtf8());
		socket->waitForBytesWritten(-1);
	}
	else
	{
		QAbstractSocket::SocketError error = socket->error();
		QString str = socket->errorString();
		delete socket;
		socket = nullptr;
	}
}

void MainWindow::onReadyRead()
{
	QTcpSocket *socket = dynamic_cast<QTcpSocket *>(sender());
	if (socket)
	{
		while (true)
		{
			QByteArray buff;
			int buffLen = 0;
			MYBSProtocol *mp = nullptr;

			// �����ʱ������Ϊ��ָ�룬˵����ÿ��ָ��ĵ�һ�ζ�
			if (!m_tempBuff)
			{
				buff = socket->read(sizeof(MYBSProtocol));
				buffLen = buff.size();
				if(buffLen == 0)
					break;
				mp = (MYBSProtocol *)buff.data();

				// ��ֹ�ϴ��ڴ�����ʧ�ܶ�������������
				if (mp->type >= MAX)
					return;

				// ����ڴ����ʧ�ܣ�ֱ�ӹر�socket����һ�ε�����ȫ��Ҫ��
				m_tempBuff = (char *)malloc(sizeof(mp) + mp->length);
				if (!m_tempBuff)
				{
					socket->close();
					socket->waitForDisconnected(-1);
					m_surplus = 0;
					return;
				}
				memset(m_tempBuff, 0, sizeof(mp) + mp->length);

				memcpy(m_tempBuff, buff.data(), buffLen);
				m_cursor += buffLen;
				m_surplus = mp->length;
			}
			else
			{
				buff = socket->read(m_surplus > m_maxRead ? m_maxRead : m_surplus);
				buffLen = buff.size();

				// buffLen��m_surplus����0�����ݶγ���Ϊ0�����
				// ��ֹ���ݶγ��ȵ�����ʱ����ڴ�й¶
				if (buffLen == 0 && m_surplus != 0)
					break;
				memcpy(m_tempBuff + m_cursor, buff.data(), buffLen);
				m_cursor += buffLen;
				m_surplus -= buffLen;

				if (m_surplus == 0)
				{
					mp = (MYBSProtocol *)m_tempBuff;

					switch (mp->type)
					{
					case TEXT:
					{
						QByteArray ba(mp->data, mp->length);
						QString text = QString::fromLocal8Bit(ba);
						ui->browseArea->setText(text);
					}break;
					case PICTURE:
					{
						QPixmap imageresult;
						QByteArray ba(mp->data, mp->length);
						imageresult.loadFromData(ba);
						ui->browseArea->setPixmap(imageresult);
					}break;
					case END:
					{
						socket->close();
						socket->waitForDisconnected(-1);
					}break;
					default:
						break;
					}

					// ����������ж�һ���Ƿ����������һ���������
					// �����������ͷ��ڴ�
					if (m_tempBuff)
					{
						free(m_tempBuff);
						m_tempBuff = nullptr;
						m_cursor = 0;
					}
				}
			}
		}
	}
}

void MainWindow::onDisconnected()
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
