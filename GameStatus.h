#ifndef GAMESTATUS_H
#define GAMESTATUS_H

#include <QDateTime>
#include <QStringList>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class GameStatusTab;
}
QT_END_NAMESPACE

class QSettings;
class QTimer;

class GameStatusTab : public QWidget
{
    Q_OBJECT

public:
    explicit GameStatusTab(QWidget *parent = nullptr);
    ~GameStatusTab();

    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();

private slots:
    void onAddGameClicked();
    void onRemoveGameClicked();
    void onSaveClicked();
    void onResetClicked();
    void onApplyClicked();
    void onDemoModeToggled(bool enabled);
    void onDemoTick();
    void onScanIntervalChanged(int value);
    void onSceneMappingChanged();

private:
    void populateSceneComboBoxes();
    void updateGameList();
    QStringList getMonitoredGames() const;
    void setMonitoredGames(const QStringList &games);
    void startDemo();
    void stopDemo();
    void updateTelemetry();
    void appendScanLog(const QString &message);
    void updateMappingSummary();
    QStringList demoGamePool() const;
    void setRunState(bool running);

    Ui::GameStatusTab *ui;
    QSettings *m_settings;
    QTimer *m_demoTimer;
    int m_timeToNextScanMs;
    bool m_isRunning;
    QString m_currentGame;
    QDateTime m_lastScan;
};

#endif // GAMESTATUS_H
