#include <QCoreApplication>
#include <QString>

#include "chatserver.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // Порт по умолчанию.
    quint16 port = 5000;

    // Если пользователь передал порт в командной строке:
    // ./chat-server 8080
    if (argc > 1) {
        bool ok = false;
        const quint16 userPort =
                QString::fromLocal8Bit(argv[1]).toUShort(&ok);

        if (ok) {
            port = userPort;
        }
    }

    ChatServer server;

    if (!server.start(port)) {
        return 1;
    }

    // Запускаем цикл обработки событий.
    return app.exec();
}