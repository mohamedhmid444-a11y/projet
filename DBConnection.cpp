#include "DBConnection.h"

#include <QDir>
#include <QSqlQuery>
#include <QStandardPaths>

namespace {
const char *kConnectionName = "smart_switcher_connection";
const char *kDatabaseFile = "smart_switcher.db";
}

/*
XAMPP setup instructions (for MySQL builds):
1) Install XAMPP and start the MySQL service.
2) Create a database named "smart_switcher".
3) Ensure MySQL is running on localhost with user "root" and empty password.
4) Make sure the Qt MySQL plugin (QMYSQL) is available.

Note: This build uses SQLite (QSQLITE) by default so it runs without extra installs.
*/

DBConnection &DBConnection::instance()
{
    static DBConnection instance;
    return instance;
}

DBConnection::DBConnection() = default;

DBConnection::~DBConnection()
{
    if (m_db.isValid() && m_db.isOpen()) {
        m_db.close();
    }
}

bool DBConnection::initialize()
{
    if (m_db.isValid() && m_db.isOpen()) {
        return true;
    }

    if (QSqlDatabase::contains(kConnectionName)) {
        m_db = QSqlDatabase::database(kConnectionName);
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
    }

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_db.setDatabaseName(QDir(dataDir).filePath(kDatabaseFile));

    if (!m_db.open()) {
        return false;
    }

    return ensureUsersTable();
}

QSqlDatabase DBConnection::database() const
{
    return m_db;
}

bool DBConnection::ensureUsersTable()
{
    QSqlQuery query(m_db);
    return query.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "email TEXT NOT NULL,"
        "password TEXT NOT NULL"
        ")");
}
