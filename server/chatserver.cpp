#include "chatserver.h"
#include "protocol.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QDebug>
#include <QList>
#include <QString>

ChatServer::ChatServer(QObject* parent)
    : QTcpServer(parent)
{
    // Когда приходит новое подключение,
    // вызываем метод onNewConnection().
    connect(this, &QTcpServer::newConnection,
            this, &ChatServer::onNewConnection);
}

bool ChatServer::start(quint16 port)
{
    // Слушаем все сетевые интерфейсы на указанном порту.
    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Cannot listen on port"
                    << port
                    << ":"
                    << errorString();
        return false;
    }

    qDebug() << "Chat server started on port" << port;
    return true;
}

void ChatServer::onNewConnection()
{
    // Забираем все ожидающие подключения.
    while (QTcpSocket* socket = nextPendingConnection()) {

        // Пока имя клиента неизвестно.
        m_clients.insert(socket, QString());

        // Когда придут данные - вызовем onReadyRead().
        connect(socket, &QTcpSocket::readyRead,
                this, &ChatServer::onReadyRead);

        // Когда клиент отключится - вызовем onDisconnected().
        connect(socket, &QTcpSocket::disconnected,
                this, &ChatServer::onDisconnected);

        // Отправляем приветствие.
        sendLine(socket, chat::protocol::makeWelcome());
    }
}

void ChatServer::onReadyRead()
{
    qDebug() << "READY_READ";
    // Получаем сокет, который прислал сигнал.
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    // Читаем все полностью принятые строки.
    while (socket->canReadLine()) {
        const QByteArray line = socket->readLine().trimmed();

        if (!line.isEmpty()) {
            processLine(socket, line);
        }
    }
}

void ChatServer::onDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    // Удаляем клиента из списка.
    const QString name = m_clients.take(socket);

    // Если клиент успел задать имя, сообщаем всем, что он вышел.
    if (!name.isEmpty()) {
        broadcast(chat::protocol::makeLeft(name));
    }

    // Разрешаем Qt удалить объект сокета позже.
    socket->deleteLater();
}

void ChatServer::processLine(QTcpSocket* socket, const QByteArray& line)
{
    // Переводим байты в строку.
    const QString text = QString::fromUtf8(line);
    qDebug() << "SOMEBODY_PUT_TEXT: " << text;
    // Разбираем строку на команду и аргумент.
    const chat::protocol::ParsedLine parsed =
            chat::protocol::parseLine(text);

    if (parsed.command == chat::protocol::nameCommand()) {

        // Клиент отправил:
        // NAME Alice

        if (parsed.argument.isEmpty()) {
            sendLine(socket,
                     chat::protocol::makeError("Empty name"));
            return;
        }

        const QString oldName = nameOf(socket);

        // Сохраняем имя.
        m_clients[socket] = parsed.argument;

        // Подтверждаем клиенту.
        sendLine(socket,
                 chat::protocol::makeOk("NAME " + parsed.argument));

        if (oldName.isEmpty()) {
            // Клиент впервые задал имя.
            broadcast(chat::protocol::makeJoin(parsed.argument));
        } else {
            // Клиент сменил имя.
            broadcast(chat::protocol::makeLeft(oldName));
            broadcast(chat::protocol::makeJoin(parsed.argument));
        }

    } else if (parsed.command == chat::protocol::messageCommand()) {

        // Клиент отправил:
        // MESSAGE Hello

        const QString name = nameOf(socket);

        if (name.isEmpty()) {
            sendLine(socket,
                     chat::protocol::makeError(
                         "Set your name first: NAME <name>"));
            return;
        }

        if (parsed.argument.isEmpty()) {
            sendLine(socket,
                     chat::protocol::makeError("Empty message"));
            return;
        }

        // Рассылаем сообщение всем клиентам.
        broadcast(chat::protocol::makeChat(name, parsed.argument));

    } else {
        sendLine(socket,
                 chat::protocol::makeError("Unknown command"));
    }
}

void ChatServer::broadcast(const QString& text, QTcpSocket* exclude)
{
    // Получаем список всех сокетов.
    const QList<QTcpSocket*> clients = m_clients.keys();

    // Проходим по всем клиентам.
    for (QTcpSocket* client : clients) {

        // Этот клиент можно исключить из рассылки.
        if (client == exclude) {
            continue;
        }

        // Отправляем только подключённым клиентам.
        if (client->state() == QAbstractSocket::ConnectedState) {
            sendLine(client, text);
        }
    }
}

void ChatServer::sendLine(QTcpSocket* socket, const QString& text)
{
    if (!socket) {
        return;
    }

    if (socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    // Преобразуем строку в UTF-8.
    QByteArray data = text.toUtf8();

    // Добавляем конец строки.
    data.append('\n');

    // Отправляем данные клиенту.
    socket->write(data);

    // Сразу просим отправить данные,
    // не дожидаясь следующей итерации цикла событий.
    socket->flush();
}

QString ChatServer::nameOf(QTcpSocket* socket) const
{
    return m_clients.value(socket);
}
