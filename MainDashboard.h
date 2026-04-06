#ifndef MAINDASHBOARD_H
#define MAINDASHBOARD_H

#include <QPointer>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainDashboard;
}
QT_END_NAMESPACE

class QDialog;

class MainDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit MainDashboard(const QString &username, QWidget *parent = nullptr);
    ~MainDashboard();

private slots:
    void onLogoutClicked();
    void onStartPluginClicked();
    void onStopPluginClicked();
    void onConfigureAudio();
    void onConfigureFace();
    void onConfigureActiveWindow();
    void onConfigureGameStatus();
    void onConfigureGeneral();

private:
    void setStatus(const QString &status, const QString &color);
    void showFeatureDialog(const QString &featureName);

    Ui::MainDashboard *ui;
    QString m_username;
    QPointer<QDialog> m_gameStatusDialog;
};

#endif // MAINDASHBOARD_H
