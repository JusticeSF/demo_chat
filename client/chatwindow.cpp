#include "chatwindow.h"
#include "protocol.h"

#include <QTcpSocket>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

ChatWindow::ChatWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Chat Client"));
    resize(700, 500);

    // Создаём сокет.
    // Родитель - это окно, поэтому окно позаботится об удалении.
    m_socket = new QTcpSocket(this);

    // Создаём элементы интерфейса.
    m_hostEdit = new QLineEdit(QStringLiteral("127.0.0.1"));
    m_portEdit = new QLineEdit(QStringLiteral("5000"));
    m_nameEdit = new QLineEdit(QStringLiteral("student"));
    m_messageEdit = new QLineEdit();
    m_logEdit = new QTextEdit();
    m_logEdit->setReadOnly(true);

    m_connectButton = new QPushButton(QStringLiteral("Connect"));
    m_sendButton = new QPushButton(QStringLiteral("Send"));

    // Верхняя панель: хост, порт, имя, кнопка подключения.
    QHBoxLayout* connectionLayout = new QHBoxLayout();
    connectionLayout->addWidget(new QLabel(QStringLiteral("Host")));
    connectionLayout->addWidget(m_hostEdit);
    connectionLayout->addWidget(new QLabel(QStringLiteral("Port")));
    connectionLayout->addWidget(m_portEdit);
    connectionLayout->addWidget(new QLabel(QStringLiteral("Name")));
    connectionLayout->addWidget(m_nameEdit);
    connectionLayout->addWidget(m_connectButton);

    // Нижняя панель: сообщение и кнопка отправки.
    QHBoxLayout* messageLayout = new QHBoxLayout();
    messageLayout->addWidget(m_messageEdit);
    messageLayout->addWidget(m_sendButton);

    // Главный вертикальный макет.
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addLayout(connectionLayout);
    mainLayout->addWidget(m_logEdit);
    mainLayout->addLayout(messageLayout);
    setLayout(mainLayout);

    // Кнопки и поле ввода сообщения.
    connect(m_connectButton, &QPushButton::clicked,
            this, &ChatWindow::onConnectClicked);

    connect(m_sendButton, &QPushButton::clicked,
            this, &ChatWindow::onSendClicked);

    connect(m_messageEdit, &QLineEdit::returnPressed,
            this, &ChatWindow::onSendClicked);

    // Сетевые события.
    connect(m_socket, &QTcpSocket::connected,
            this, &ChatWindow::onConnected);

    connect(m_socket, &QTcpSocket::disconnected,
            this, &ChatWindow::onDisconnected);

    connect(m_socket, &QTcpSocket::readyRead,
            this, &ChatWindow::onReadyRead);

    // В старых версиях Qt сигнал error перегружен.
    // Для совместимости используем старый стиль записи.
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onSocketError(QAbstractSocket::SocketError)));
}

void ChatWindow::onConnectClicked()
{
    // Читаем данные из полей интерфейса.
    const QString host = m_hostEdit->text().trimmed();
    const quint16 port = m_portEdit->text().toUShort();
    const QString name = m_nameEdit->text().trimmed();

    if (host.isEmpty()) {
        appendText(QStringLiteral("Host is empty"));
        return;
    }

    if (port == 0) {
        appendText(QStringLiteral("Invalid port"));
        return;
    }

    if (name.isEmpty()) {
        appendText(QStringLiteral("Name is empty"));
        return;
    }

    // Если сокет уже в каком-то процессе соединения,
    // сбрасываем его состояние.
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }

    appendText(QStringLiteral("Connecting to %1:%2...")
                   .arg(host)
                   .arg(QString::number(port)));

    // Подключаемся к серверу.
    m_socket->connectToHost(host, port);
}

void ChatWindow::onSendClicked()
{
    const QString message = m_messageEdit->text().trimmed();

    if (message.isEmpty()) {
        return;
    }

    if (!isConnected()) {
        appendText(QStringLiteral("Not connected"));
        return;
    }

    // Отправляем команду MESSAGE.
    sendLine(chat::protocol::makeMessageCommand(message));

    // Очищаем поле ввода.
    m_messageEdit->clear();
}

void ChatWindow::onConnected()
{
    appendText(QStringLiteral("Connected"));

    const QString name = m_nameEdit->text().trimmed();

    if (!name.isEmpty()) {
        // Сразу отправляем имя после подключения.
        sendLine(chat::protocol::makeNameCommand(name));
    }
}

void ChatWindow::onDisconnected()
{
    appendText(QStringLiteral("Disconnected"));
}

void ChatWindow::onReadyRead()
{
    // Читаем все готовые строки от сервера.
    while (m_socket->canReadLine()) {
        const QByteArray line = m_socket->readLine().trimmed();

        if (!line.isEmpty()) {
            appendText(QString::fromUtf8(line));
        }
    }
}

void ChatWindow::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    appendText(QStringLiteral("Socket error: %1")
                   .arg(m_socket->errorString()));
}

void ChatWindow::appendText(const QString& text)
{
    m_logEdit->append(text);
}

void ChatWindow::sendLine(const QString& text)
{
    if (!isConnected()) {
        return;
    }

    QByteArray data = text.toUtf8();
    data.append('\n');

    m_socket->write(data);
    m_socket->flush();
}

bool ChatWindow::isConnected() const
{
    return m_socket &&
           m_socket->state() == QAbstractSocket::ConnectedState;
}