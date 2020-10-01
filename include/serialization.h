#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <QJsonArray>
#include <QJsonObject>


struct Snowflake
{

};

struct Emoji
{
    Snowflake id;
    QString name;
    bool isAnimated;
};

struct Party
{
    QString id;
    int size;
    int maxSize;
};

struct Assets
{
    QString largeImage;
    QString largeText;
    QString smallImage;
    QString smallText;
};

struct Secrets
{
    QString join;
    QString spectate;
    QString match;
};

struct Activity
{
    QString name;

    enum class Type
    {
        Game,
        Streaming,
        Listening,
        Custom,
        Competing
    };

    Type type;
    QUrl steamUrl;
    QDateTime createdAt;

    QDateTime startTime;
    QDateTime endTime;

    Snowflake applicationId;

    QString details;
    QString state;

    Emoji emoji;

    Party party;

    Assets assets;

    Secrets secrets;

    bool isInstance;

    enum class ActivityFlags
    {
        Instance    = 1 << 0,
        Join        = 1 << 1,
        Spectate    = 1 << 2,
        JoinRequest = 1 << 3,
        Sync        = 1 << 4,
        Play        = 1 << 5
    };

    int activityFlags;

    QJsonObject toJson();
};

struct Presence
{
    bool isAfk;
    QString status;
    QList<Activity> activities;

    QJsonObject toJson();
};

#endif // SERIALIZATION_H
