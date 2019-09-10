#include "websocketclient.h"
#include <QtCore/QDebug>

WebsocketClient::WebsocketClient(const QUrl &url, bool debug, QObject *parent):
    QObject(parent),
    m_url(url),
    m_debug(debug),
    m_connected(false)
{
    if (m_debug)
        qDebug() << "WebSocket server:" << url;
    connect(&m_webSocket, &QWebSocket::connected, this, &WebsocketClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &WebsocketClient::closed);
    connect(&m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    m_webSocket.open(QUrl(url));
    m_webSocket.ping();
}

//! [onConnected]
void WebsocketClient::onConnected()
{
    if (m_debug)
        qDebug() << "WebSocket connected";
    // 感觉这样多此一举，转了一道啊，感觉可以直接对应到 gui中的槽中啊
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &WebsocketClient::onTextMessageReceived);
    m_connected = true;
    m_webSocket.sendTextMessage(QStringLiteral("Hello, world!"));
}

//! [onTextMessageReceived]
void WebsocketClient::onTextMessageReceived(const QString message)
{
    if (m_debug)
        qDebug() << "Message received:" << message;
    //m_webSocket.close();
    m_webSocket.sendTextMessage(QStringLiteral("Hello, world!"));
    emit sendMsg(message);
}

void WebsocketClient::error(QAbstractSocket::SocketError error)
{
    QString e;
    e.sprintf("error: %d",error);
    emit sendMsg(e);
}

int WebsocketClient::sentMessage(const QString &message)
{
    if (m_connected) {
        return m_webSocket.sendTextMessage(message);
    }

    return 0;
}

WebsocketClient::~WebsocketClient()
{
    m_webSocket.close();
}
