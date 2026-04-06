#include "MainDashboard.h"
#include "ui_MainDashboard.h"

#include "GameStatus.h"
#include "LoginWindow.h"

#include <QAbstractAnimation>
#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QVBoxLayout>

MainDashboard::MainDashboard(const QString &username, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MainDashboard),
      m_username(username)
{
    ui->setupUi(this);

    ui->labelUsername->setText(QString("Signed in as %1").arg(m_username));
    setStatus("Stopped", "#ef476f");

    auto fadeWidget = [](QWidget *widget, int delayMs) {
        if (!widget) {
            return;
        }
        auto *effect = new QGraphicsOpacityEffect(widget);
        effect->setOpacity(0.0);
        widget->setGraphicsEffect(effect);

        auto *animation = new QPropertyAnimation(effect, "opacity", widget);
        animation->setDuration(450);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::OutCubic);

        QTimer::singleShot(delayMs, widget, [animation]() {
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        });
    };

    fadeWidget(ui->topBar, 0);
    fadeWidget(ui->heroFrame, 80);
    fadeWidget(ui->labelSectionTitle, 140);
    fadeWidget(ui->cardAudio, 200);
    fadeWidget(ui->cardFace, 260);
    fadeWidget(ui->cardActiveWindow, 320);
    fadeWidget(ui->cardGameStatus, 380);
    fadeWidget(ui->cardGeneral, 440);
    fadeWidget(ui->bottomBar, 520);

    connect(ui->btnLogout, &QPushButton::clicked, this, &MainDashboard::onLogoutClicked);
    connect(ui->btnStartPlugin, &QPushButton::clicked, this, &MainDashboard::onStartPluginClicked);
    connect(ui->btnStopPlugin, &QPushButton::clicked, this, &MainDashboard::onStopPluginClicked);

    connect(ui->btnConfigureAudio, &QPushButton::clicked, this, &MainDashboard::onConfigureAudio);
    connect(ui->btnConfigureFace, &QPushButton::clicked, this, &MainDashboard::onConfigureFace);
    connect(ui->btnConfigureActiveWindow, &QPushButton::clicked, this, &MainDashboard::onConfigureActiveWindow);
    connect(ui->btnConfigureGameStatus, &QPushButton::clicked, this, &MainDashboard::onConfigureGameStatus);
    connect(ui->btnConfigureGeneral, &QPushButton::clicked, this, &MainDashboard::onConfigureGeneral);
}

MainDashboard::~MainDashboard()
{
    delete ui;
}

void MainDashboard::onLogoutClicked()
{
    auto *login = new LoginWindow();
    login->setAttribute(Qt::WA_DeleteOnClose);
    login->show();
    close();
}

void MainDashboard::onStartPluginClicked()
{
    setStatus("Running", "#06d6a0");
}

void MainDashboard::onStopPluginClicked()
{
    setStatus("Stopped", "#ef476f");
}

void MainDashboard::onConfigureAudio()
{
    showFeatureDialog("Audio Settings");
}

void MainDashboard::onConfigureFace()
{
    showFeatureDialog("Face Detection");
}

void MainDashboard::onConfigureActiveWindow()
{
    showFeatureDialog("Active Window");
}

void MainDashboard::onConfigureGameStatus()
{
    if (m_gameStatusDialog) {
        m_gameStatusDialog->raise();
        m_gameStatusDialog->activateWindow();
        return;
    }

    auto *dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("Game Status");
    dialog->resize(680, 760);

    auto *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new GameStatusTab(dialog));

    m_gameStatusDialog = dialog;
    connect(dialog, &QDialog::finished, this, [this]() {
        m_gameStatusDialog = nullptr;
    });

    dialog->show();
}

void MainDashboard::onConfigureGeneral()
{
    showFeatureDialog("General Settings");
}

void MainDashboard::setStatus(const QString &status, const QString &color)
{
    ui->labelStatus->setText(QString("Status: %1").arg(status));
    ui->labelStatus->setStyleSheet(QString("color: %1; font-weight: 600;").arg(color));
    ui->statusDot->setStyleSheet(QString("background-color: %1; border-radius: 6px;").arg(color));
}

void MainDashboard::showFeatureDialog(const QString &featureName)
{
    QMessageBox::information(this, featureName, "Configure this feature in a future update.");
}
