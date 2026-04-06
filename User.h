#ifndef USER_H
#define USER_H

#include <QString>

class User
{
public:
    User();

    int id() const;
    void setId(int id);

    QString username() const;
    void setUsername(const QString &username);

    QString email() const;
    void setEmail(const QString &email);

    QString password() const;
    void setPassword(const QString &password);

    bool create();
    static bool findByUsername(const QString &username, User *outUser);
    static bool validateLogin(const QString &username, const QString &password, User *outUser = nullptr);

private:
    static QString hashPassword(const QString &password);

    int m_id;
    QString m_username;
    QString m_email;
    QString m_password;
};

#endif // USER_H
