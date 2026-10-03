#pragma once

#include <QString>

namespace chat {
namespace protocol {

// Команда, которой клиент задаёт имя.
inline QString nameCommand()
{
    return QStringLiteral("NAME");
}

// Команда, которой клиент отправляет сообщение.
inline QString messageCommand()
{
    return QStringLiteral("MESSAGE");
}

// Результат разбора одной строки протокола.
struct ParsedLine {
    QString command;
    QString argument;
};

// Разбирает строку вида:
// NAME Alice
// MESSAGE Hello world
inline ParsedLine parseLine(const QString& line)
{
    ParsedLine parsed;

    // Убираем лишние пробелы в начале и в конце.
    const QString text = line.trimmed();

    // Ищем первый пробел.
    const int spacePos = text.indexOf(QLatin1Char(' '));

    if (spacePos == -1) {
        // Пробела нет: вся строка - это команда.
        parsed.command = text;
    } else {
        // До пробела - команда, после пробела - аргумент.
        parsed.command = text.left(spacePos);
        parsed.argument = text.mid(spacePos + 1).trimmed();
    }

    return parsed;
}

// Создать команду вида:
// NAME Alice
inline QString makeNameCommand(const QString& name)
{
    return nameCommand() + QLatin1Char(' ') + name;
}

// Создать команду вида:
// MESSAGE Hello
inline QString makeMessageCommand(const QString& message)
{
    return messageCommand() + QLatin1Char(' ') + message;
}

// Приветствие сервера.
inline QString makeWelcome()
{
    return QStringLiteral("WELCOME Use NAME <name> and MESSAGE <text>");
}

// Успешный ответ сервера.
inline QString makeOk(const QString& text)
{
    return QStringLiteral("OK ") + text;
}

// Ответ с ошибкой.
inline QString makeError(const QString& text)
{
    return QStringLiteral("ERROR ") + text;
}

// Сообщение о том, что пользователь вошёл.
inline QString makeJoin(const QString& name)
{
    return QStringLiteral("JOIN ") + name;
}

// Сообщение о том, что пользователь вышел.
inline QString makeLeft(const QString& name)
{
    return QStringLiteral("LEFT ") + name;
}

// Сообщение чата вида:
// CHAT Alice: Hello
inline QString makeChat(const QString& name, const QString& text)
{
    return QStringLiteral("CHAT ") + name + QStringLiteral(": ") + text;
}

} // namespace protocol
} // namespace chat