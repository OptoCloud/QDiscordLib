#include "DiscordClient.h"

#include <QDebug>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>

#include "serialization.h"

#define urlAPI "https://discord.com/api/v6"
#define urlCDN "https://cdn.discordapp.com/"

const char* libName = "QDiscordLib";

QNetworkRequest CreateRequest(QUrl url)
{
    QNetworkRequest request;

    request.setUrl(url);
    request.setRawHeader("User-Agent", libName);
    return request;
}


QDiscordLib::DiscordClient::DiscordClient(QObject* parent)
    : QObject(parent)
    , m_gatewayUrl()
    , m_networkManager(new QNetworkAccessManager(this))
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_heartbeatTimer(new QTimer(this))
{
    m_authKey = "";

    // Setup network manager
    connect(m_networkManager, &QNetworkAccessManager::sslErrors, this, &DiscordClient::onSslError);

    // Setup web socket
    connect(m_webSocket, &QWebSocket::connected, this, &DiscordClient::onGatewayConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &DiscordClient::onGatewayDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &DiscordClient::onGatewayMessage);

    // Setup heartbeat timer
    connect(m_heartbeatTimer, &QTimer::timeout, this, &DiscordClient::sendHeartbeat);
}

QDiscordLib::DiscordClient::~DiscordClient() {
}


// Connection handling
void QDiscordLib::DiscordClient::ConnectToDiscord()
{
    if(!m_gatewayUrl.isEmpty())
    {
        m_webSocket->open(m_gatewayUrl);
        return;
    }

    // Create gateway request
    QNetworkReply* gatewayReply = m_networkManager->get(CreateRequest(QUrl(urlAPI "/gateway")));

    // Connect error handling
    connect(gatewayReply, &QNetworkReply::errorOccurred, [&](QNetworkReply::NetworkError) {
        qDebug() << "[QDiscordLib] Caught error when sending gateway request!";
        qDebug() << "[QDiscordLib] Error:" << gatewayReply->errorString();
    });

    // Handle gateway response
    connect(gatewayReply, &QIODevice::readyRead, [gatewayReply, this]() {

        QByteArray gatewayBytes = gatewayReply->readAll();

        if(gatewayBytes.isEmpty()) {
            emit onError("Gateway response returned an empty byte array!");
            return;
        }

        // Parse JSON response
        QJsonValue gatewayUrl = QJsonDocument::fromJson(gatewayBytes).object().value("url");
        if(gatewayUrl.isUndefined() || !gatewayUrl.isString()) {
            emit onError("Failed to parse gateway response!");
            return;
        }

        // Open the web sockets
        m_gatewayUrl = gatewayUrl.toString() + "/?v=6&encoding=json";
        qDebug() << "[QDiscordLib] Found gateway url" << m_gatewayUrl;
        m_webSocket->open(QUrl(m_gatewayUrl));
    });
}

void QDiscordLib::DiscordClient::onSslError(QNetworkReply* reply, const QList<QSslError>& errors)
{
    emit onError("[QDiscordLib] SSL error when using network manager!");
    for(QSslError err : errors)
    {
        qDebug() << "[QDiscordLib] SSL Error:" << err.error();
    }
}

void QDiscordLib::DiscordClient::onGatewayConnected()
{
    qDebug() << "[QDiscordLib] Connected to Discord gateway!";
    emit onReady();
}

void QDiscordLib::DiscordClient::onGatewayDisconnected()
{
    qWarning() << "[QDiscordLib] Gateway disconnected! Cleaning up for reconnect...";
    m_webSocket->close();

    // Emit the signal for reconnect
    emit onDisconnected();
}

enum OpCodes
{
    OpCodes_Dispatch = 0,
    OpCodes_Heartbeat = 1,
    OpCodes_Identify = 2,
    OpCodes_PresenceUpdate = 3,
    OpCodes_VoiceStateUpdate = 4,
    OpCodes_Resume = 6,
    OpCodes_Reconnect = 7,
    OpCodes_RequestGuildMembers = 8,
    OpCodes_InvalidSession = 9,
    OpCodes_Hello = 10,
    OpCodes_HeartbeatAck = 11,
};

// Gatway event handlers
void QDiscordLib::DiscordClient::onGatewayMessage(const QString& message)
{
    QJsonParseError err;
    QJsonObject json = QJsonDocument::fromJson(message.toUtf8(), &err).object();

    if (json.isEmpty())
    {
        return;
    }

    // Get the STD's
    QJsonObject s = json["s"].toObject();
    QJsonObject t = json["t"].toObject();
    QJsonObject d = json["d"].toObject();

    switch (json["op"].toInt())
    {
    case OpCodes_Dispatch:
        qDebug() << "OPCODE: Dispatch";
        qDebug().noquote() << QJsonDocument(json).toJson(QJsonDocument::Indented);
        break;
    case OpCodes_Heartbeat:
        qDebug() << "OPCODE: Heartbeat";
        break;
    case OpCodes_Identify:
        qDebug() << "OPCODE: Identify";
        break;
    case OpCodes_PresenceUpdate:
        qDebug() << "OPCODE: PresenceUpdate";
        break;
    case OpCodes_VoiceStateUpdate:
        qDebug() << "OPCODE: VoiceStateUpdate";
        break;
    case OpCodes_Resume:
        qDebug() << "OPCODE: Resume";
        break;
    case OpCodes_Reconnect:
        qDebug() << "OPCODE: Reconnect";
        break;
    case OpCodes_RequestGuildMembers:
        qDebug() << "OPCODE: RequestGuildMembers";
        break;
    case OpCodes_InvalidSession:
        qDebug() << "OPCODE: InvalidSession";
        break;
    case OpCodes_Hello:
        qDebug() << "OPCODE: Hello";

        m_heartbeatTimer->setSingleShot(false);
        m_heartbeatTimer->start(d["heartbeat_interval"].toInt());

        identify();
        break;
    case OpCodes_HeartbeatAck:
        qDebug() << "OPCODE: HeartbeatAck";
        break;
    default:
        return;
    }

}

void QDiscordLib::DiscordClient::identify()
{
    QJsonObject properties;
#ifdef __WIN32
    properties["$os"] = "windows";
#else
    properties["$os"] = linux";
#endif
    properties["$browser"] = libName;
    properties["$device"] = libName;

    Activity activity;
    activity.type = Activity::Type::Game;
    activity.name = "Testing...";

    Presence presence;
    presence.isAfk = false;
    presence.status = "UwU";
    presence.activities.append(activity);

    QJsonObject d;
    d["token"] = "Bot NzYxMjc2MDQzMTY4MDU1MzY2.X3YPkA.SrF3RZkJuYYXeTTd9gr6b3dwOfE";
    d["properties"] = properties;
    d["compress"] = false;
    d["guild_subscriptions"] = true;
    d["presence"] = activity.toJson();
    d["intent"] = 1<<12 | 1<<10 | 1<<9 | 1<<8 | 1<<3 | 1<<2 | 1<<1 | 1;


    QJsonObject obj;
    obj["op"] = (int)OpCodes_Identify;
    obj["d"] = d;

    qDebug().noquote() << QJsonDocument(obj).toJson(QJsonDocument::Indented);

    m_webSocket->sendTextMessage(QJsonDocument(obj).toJson());

    qDebug() << "Sent identification!";
}

void QDiscordLib::DiscordClient::sendHeartbeat()
{
    QJsonObject obj;
    obj["op"] = OpCodes_Heartbeat;

    m_webSocket->sendTextMessage(QJsonDocument(obj).toJson());

    qDebug() << "Sent heatbeat!";
}
