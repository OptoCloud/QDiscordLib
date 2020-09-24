#ifndef DISCORDCLIENT_H
#define DISCORDCLIENT_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QTimer>

class QSslError;
class QJsonObject;
class QWebSocket;
class QNetworkReply;
class QNetworkRequest;
class QNetworkAccessManager;

namespace QDiscordLib {
class DiscordClient: public QObject
{
    Q_OBJECT
public:
    DiscordClient(QObject* parent = nullptr);
    ~DiscordClient();

signals:
    void onReady();
    void onConnected();
    void onDisconnected();
    void onError(const QString& error);

public slots:
    void ConnectToDiscord();

private slots:
    void onSslError(QNetworkReply* reply, const QList<QSslError>& errors);

    void onGatewayConnected();
    void onGatewayDisconnected();
    void onGatewayMessage(const QString& message);

    void identify();
    void sendHeartbeat();

private:
    QString m_authKey;

    QUrl m_gatewayUrl;
    QNetworkAccessManager* m_networkManager;
    QWebSocket* m_webSocket;
    QTimer* m_heartbeatTimer;
};
}

#endif // DISCORDCLIENT_H
