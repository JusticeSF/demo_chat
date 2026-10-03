#pragma once

#include <QTcpServer>
#include <QHash>
#include <QString>

class QTcpSocket;

class ChatServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit ChatServer(QObject* parent = nullptr);

    // Запускает сервер на указанном порту.
    bool start(quint16 port);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    // Обрабатывает одну команду клиента.
    void processLine(QTcpSocket* socket, const QByteArray& line);

    // Отправляет текст всем подключённым клиентам.
    // Если указан exclude, этот клиент не получит сообщение.
    void broadcast(const QString& text, QTcpSocket* exclude = nullptr);

    // Отправляет одну строку конкретному клиенту.
    void sendLine(QTcpSocket* socket, const QString& text);

    // Возвращает имя клиента по сокету.
    QString nameOf(QTcpSocket* socket) const;

    // Список клиентов:
    // ключ - сокет клиента,
    // значение - имя клиента.
    QHash<QTcpSocket*, QString> m_clients;
};