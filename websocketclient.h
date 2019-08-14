#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H
#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QAbstractSocket>

class WebsocketClient: public QObject
{
    Q_OBJECT
public:
    WebsocketClient(const QUrl &url, bool debug = false, QObject *parent = nullptr);
    ~WebsocketClient();

    int sentMessage(const QString &message);
Q_SIGNALS:
    void closed();
    void sendMsg(const QString&);

private Q_SLOTS:
    void onConnected();
    void onTextMessageReceived(QString message);
    void error(QAbstractSocket::SocketError error);

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;
    bool m_connected;
};

#endif // WEBSOCKETCLIENT_H
