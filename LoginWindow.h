#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginWindow;
}
QT_END_NAMESPACE

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginClicked();
    void onSignupClicked();
    void onShowSignupClicked();
    void onShowLoginClicked();

private:
    void showError(const QString &title, const QString &message);

    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
