#include "LoginWindow.h"
#include "ui_LoginWindow.h"

#include "MainDashboard.h"
#include "User.h"

#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    ui->stackedPages->setCurrentWidget(ui->pageLogin);

    connect(ui->btnLogin, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(ui->btnSignup, &QPushButton::clicked, this, &LoginWindow::onSignupClicked);
    connect(ui->btnShowSignup, &QPushButton::clicked, this, &LoginWindow::onShowSignupClicked);
    connect(ui->btnShowLogin, &QPushButton::clicked, this, &LoginWindow::onShowLoginClicked);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::onLoginClicked()
{
    const QString username = ui->editLoginUsername->text().trimmed();
    const QString password = ui->editLoginPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        showError("Missing Information", "Please enter both username and password.");
        return;
    }

    User user;
    if (!User::validateLogin(username, password, &user)) {
        showError("Login Failed", "Invalid username or password.");
        return;
    }

    auto *dashboard = new MainDashboard(user.username());
    dashboard->setAttribute(Qt::WA_DeleteOnClose);
    dashboard->show();

    close();
}

void LoginWindow::onSignupClicked()
{
    const QString username = ui->editSignupUsername->text().trimmed();
    const QString email = ui->editSignupEmail->text().trimmed();
    const QString password = ui->editSignupPassword->text();
    const QString confirm = ui->editSignupConfirm->text();

    if (username.isEmpty() || email.isEmpty() || password.isEmpty() || confirm.isEmpty()) {
        showError("Missing Information", "Please complete all signup fields.");
        return;
    }

    if (password != confirm) {
        showError("Password Mismatch", "Passwords do not match.");
        return;
    }

    User existing;
    if (User::findByUsername(username, &existing)) {
        showError("Username Taken", "That username is already in use.");
        return;
    }

    User user;
    user.setUsername(username);
    user.setEmail(email);
    user.setPassword(password);

    if (!user.create()) {
        showError("Signup Failed", "Unable to create account. Please try again.");
        return;
    }

    QMessageBox::information(this, "Account Created", "Your account is ready. Please sign in.");
    ui->stackedPages->setCurrentWidget(ui->pageLogin);
    ui->editLoginUsername->setText(username);
    ui->editLoginPassword->clear();
}

void LoginWindow::onShowSignupClicked()
{
    ui->stackedPages->setCurrentWidget(ui->pageSignup);
}

void LoginWindow::onShowLoginClicked()
{
    ui->stackedPages->setCurrentWidget(ui->pageLogin);
}

void LoginWindow::showError(const QString &title, const QString &message)
{
    QMessageBox::warning(this, title, message);
}
