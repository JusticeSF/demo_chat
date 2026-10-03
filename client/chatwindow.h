#pragma once

#include <QWidget>
#include <QAbstractSocket>

class QTcpSocket;
class QLineEdit;
class QTextEdit;
class QPushButton;

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget* parent = nullptr);

private slots:
    void onConnectClicked();
    void onSendClicked();

    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    void appendText(const QString& text);
    void sendLine(const QString& text);
    bool isConnected() const;

    // Сетевой сокет.
    QTcpSocket* m_socket;

    // Поля ввода.
    QLineEdit* m_hostEdit;
    QLineEdit* m_portEdit;
    QLineEdit* m_nameEdit;
    QLineEdit* m_messageEdit;

    // Текстовый лог.
    QTextEdit* m_logEdit;

    // Кнопки.
    QPushButton* m_connectButton;
    QPushButton* m_sendButton;
};