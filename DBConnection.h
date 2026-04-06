#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <QSqlDatabase>

class DBConnection
{
public:
    static DBConnection &instance();

    bool initialize();
    QSqlDatabase database() const;

private:
    DBConnection();
    ~DBConnection();

    DBConnection(const DBConnection &) = delete;
    DBConnection &operator=(const DBConnection &) = delete;

    bool ensureUsersTable();

    QSqlDatabase m_db;
};

#endif // DBCONNECTION_H
