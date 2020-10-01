#include "serialization.h"


QJsonObject Presence::toJson()
{
    QJsonObject obj;

    obj["afk"] = isAfk;
    obj["status"] = status;

    QJsonArray array;
    for (Activity activity : activities)
    {
        array.append(activity.toJson());
    }
    obj["activities"] = array;

    return obj;
}

QJsonObject Activity::toJson()
{
    QJsonObject obj;
    obj["name"] = name;
    obj["type"] = (int)type;

    return obj;
}
