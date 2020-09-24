#include <QCoreApplication>

#include "DiscordClient.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QDiscordLib::DiscordClient client(&app);
    client.ConnectToDiscord();

    return app.exec();
}
