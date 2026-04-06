#include "User.h"

#include "DBConnection.h"

#include <QCryptographicHash>
#include <QSqlQuery>
#include <QVariant>

User::User()
    : m_id(0)
{
}

int User::id() const
{
    return m_id;
}

void User::setId(int id)
{
    m_id = id;
}

QString User::username() const
{
    return m_username;
}

void User::setUsername(const QString &username)
{
    m_username = username;
}

QString User::email() const
{
    return m_email;
}

void User::setEmail(const QString &email)
{
    m_email = email;
}

QString User::password() const
{
    return m_password;
}

void User::setPassword(const QString &password)
{
    m_password = password;
}

bool User::create()
{
    if (m_username.isEmpty() || m_email.isEmpty() || m_password.isEmpty()) {
        return false;
    }

    if (!DBConnection::instance().initialize()) {
        return false;
    }

    QSqlQuery query(DBConnection::instance().database());
    query.prepare("INSERT INTO users (username, email, password) VALUES (?, ?, ?)");
    query.addBindValue(m_username);
    query.addBindValue(m_email);
    query.addBindValue(hashPassword(m_password));

    if (!query.exec()) {
        return false;
    }

    m_id = query.lastInsertId().toInt();
    return true;
}

bool User::findByUsername(const QString &username, User *outUser)
{
    if (!DBConnection::instance().initialize()) {
        return false;
    }

    QSqlQuery query(DBConnection::instance().database());
    query.prepare("SELECT id, username, email, password FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec()) {
        return false;
    }

    if (!query.next()) {
        return false;
    }

    if (outUser) {
        outUser->m_id = query.value(0).toInt();
        outUser->m_username = query.value(1).toString();
        outUser->m_email = query.value(2).toString();
        outUser->m_password = query.value(3).toString();
    }

    return true;
}

bool User::validateLogin(const QString &username, const QString &password, User *outUser)
{
    User user;
    if (!findByUsername(username, &user)) {
        return false;
    }

    const QString hashed = hashPassword(password);
    if (user.password() != hashed) {
        return false;
    }

    if (outUser) {
        *outUser = user;
    }

    return true;
}

QString User::hashPassword(const QString &password)
{
    const QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromLatin1(hash.toHex());
}
