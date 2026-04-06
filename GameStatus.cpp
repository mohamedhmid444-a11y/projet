#include "GameStatus.h"
#include "ui_GameStatus.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTime>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSettings>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>

namespace {
const char *kOrgName = "SmartSceneSwitcher";
const char *kAppName = "GameStatus";
const char *kKeyMonitoredGames = "monitoredGames";
const char *kKeySceneGameRunning = "sceneGameRunning";
const char *kKeySceneNoGame = "sceneNoGame";
const char *kKeyScanIntervalMs = "scanIntervalMs";
const char *kKeyAutoDetect = "autoDetect";
const char *kKeyDemoMode = "demoMode";
}

GameStatusTab::GameStatusTab(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::GameStatusTab),
      m_settings(new QSettings(kOrgName, kAppName, this)),
      m_demoTimer(new QTimer(this)),
      m_timeToNextScanMs(0),
      m_isRunning(false)
{
    ui->setupUi(this);

    ui->tableMappingSummary->setColumnCount(2);
    ui->tableMappingSummary->setHorizontalHeaderLabels({tr("Game"), tr("Scene")});
    ui->tableMappingSummary->horizontalHeader()->setStretchLastSection(true);
    ui->tableMappingSummary->verticalHeader()->setVisible(false);
    ui->tableMappingSummary->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableMappingSummary->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableMappingSummary->setShowGrid(false);

    ui->listScanLog->setSelectionMode(QAbstractItemView::NoSelection);

    connect(ui->btnAddGame, &QPushButton::clicked, this, &GameStatusTab::onAddGameClicked);
    connect(ui->btnRemoveGame, &QPushButton::clicked, this, &GameStatusTab::onRemoveGameClicked);
    connect(ui->btnSave, &QPushButton::clicked, this, &GameStatusTab::onSaveClicked);
    connect(ui->btnReset, &QPushButton::clicked, this, &GameStatusTab::onResetClicked);
    connect(ui->btnApply, &QPushButton::clicked, this, &GameStatusTab::onApplyClicked);
    connect(ui->listMonitoredGames, &QListWidget::itemSelectionChanged, this, &GameStatusTab::updateGameList);
    connect(ui->checkDemoMode, &QCheckBox::toggled, this, &GameStatusTab::onDemoModeToggled);
    connect(ui->spinScanInterval,
        QOverload<int>::of(&QSpinBox::valueChanged),
        this,
        &GameStatusTab::onScanIntervalChanged);
    connect(ui->comboSceneGameRunning,
        &QComboBox::currentTextChanged,
        this,
        &GameStatusTab::onSceneMappingChanged);
    connect(ui->comboSceneNoGame,
        &QComboBox::currentTextChanged,
        this,
        &GameStatusTab::onSceneMappingChanged);

    m_demoTimer->setInterval(1000);
    connect(m_demoTimer, &QTimer::timeout, this, &GameStatusTab::onDemoTick);

    populateSceneComboBoxes();
    loadSettings();
    updateGameList();
    updateTelemetry();
}

GameStatusTab::~GameStatusTab()
{
    delete ui;
}

void GameStatusTab::populateSceneComboBoxes()
{
    ui->comboSceneGameRunning->clear();
    ui->comboSceneNoGame->clear();

    const QStringList demoScenes = {
        tr("Gameplay"),
        tr("Starting Soon"),
        tr("BRB"),
        tr("Intermission"),
        tr("Just Chatting")
    };

    for (const QString &scene : demoScenes) {
        ui->comboSceneGameRunning->addItem(scene);
        ui->comboSceneNoGame->addItem(scene);
    }

    // TODO: Fetch scene list from OBS and populate the combo boxes.
    // TODO: Keep these scene lists in sync with OBS changes.
}

void GameStatusTab::loadSettings()
{
    setMonitoredGames(m_settings->value(kKeyMonitoredGames).toStringList());

    const QString sceneRunning = m_settings->value(kKeySceneGameRunning).toString();
    const QString sceneNoGame = m_settings->value(kKeySceneNoGame).toString();

    auto ensureScene = [](QComboBox *combo, const QString &sceneName) {
        if (sceneName.isEmpty()) {
            return;
        }
        int index = combo->findText(sceneName);
        if (index < 0) {
            combo->addItem(sceneName);
            index = combo->findText(sceneName);
        }
        if (index >= 0) {
            combo->setCurrentIndex(index);
        }
    };

    ensureScene(ui->comboSceneGameRunning, sceneRunning);
    ensureScene(ui->comboSceneNoGame, sceneNoGame);

    ui->checkAutoDetect->setChecked(m_settings->value(kKeyAutoDetect, false).toBool());
    ui->spinScanInterval->setValue(m_settings->value(kKeyScanIntervalMs, 2000).toInt());
    ui->checkDemoMode->setChecked(m_settings->value(kKeyDemoMode, true).toBool());
    m_timeToNextScanMs = ui->spinScanInterval->value();

    updateGameList();
    updateTelemetry();

    if (ui->checkDemoMode->isChecked()) {
        startDemo();
    } else {
        stopDemo();
    }
}

void GameStatusTab::saveSettings()
{
    m_settings->setValue(kKeyMonitoredGames, getMonitoredGames());
    m_settings->setValue(kKeySceneGameRunning, ui->comboSceneGameRunning->currentText());
    m_settings->setValue(kKeySceneNoGame, ui->comboSceneNoGame->currentText());
    m_settings->setValue(kKeyScanIntervalMs, ui->spinScanInterval->value());
    m_settings->setValue(kKeyAutoDetect, ui->checkAutoDetect->isChecked());
    m_settings->setValue(kKeyDemoMode, ui->checkDemoMode->isChecked());
    m_settings->sync();

    // TODO: Notify the game process monitor to refresh the watched process list.
}

void GameStatusTab::onAddGameClicked()
{
    bool ok = false;
    QString gameName = QInputDialog::getText(
        this,
        tr("Add Game"),
        tr("Enter process name (e.g., game.exe):"),
        QLineEdit::Normal,
        QString(),
        &ok);

    if (!ok) {
        return;
    }

    gameName = gameName.trimmed();
    if (gameName.isEmpty()) {
        return;
    }

    const QStringList existing = getMonitoredGames();
    for (const QString &game : existing) {
        if (QString::compare(game, gameName, Qt::CaseInsensitive) == 0) {
            QMessageBox::information(
                this,
                tr("Already Added"),
                tr("That game is already in the list."));
            return;
        }
    }

    ui->listMonitoredGames->addItem(gameName);
    updateGameList();
}

void GameStatusTab::onRemoveGameClicked()
{
    const QList<QListWidgetItem *> selected = ui->listMonitoredGames->selectedItems();
    if (selected.isEmpty()) {
        return;
    }

    for (QListWidgetItem *item : selected) {
        delete ui->listMonitoredGames->takeItem(ui->listMonitoredGames->row(item));
    }

    updateGameList();
}

void GameStatusTab::onSaveClicked()
{
    saveSettings();
    QMessageBox::information(this, tr("Settings Saved"), tr("Game status settings have been saved."));
}

void GameStatusTab::onResetClicked()
{
    loadSettings();
    QMessageBox::information(this, tr("Settings Reset"), tr("Game status settings have been restored."));
}

void GameStatusTab::onApplyClicked()
{
    saveSettings();
    emit settingsChanged();
    QMessageBox::information(this, tr("Settings Applied"), tr("Game status settings have been applied."));
}

void GameStatusTab::updateGameList()
{
    ui->btnRemoveGame->setEnabled(!ui->listMonitoredGames->selectedItems().isEmpty());
    updateMappingSummary();
}

QStringList GameStatusTab::getMonitoredGames() const
{
    QStringList games;
    games.reserve(ui->listMonitoredGames->count());

    for (int i = 0; i < ui->listMonitoredGames->count(); ++i) {
        games.append(ui->listMonitoredGames->item(i)->text());
    }

    return games;
}

void GameStatusTab::setMonitoredGames(const QStringList &games)
{
    ui->listMonitoredGames->clear();

    for (const QString &game : games) {
        ui->listMonitoredGames->addItem(game);
    }

    updateMappingSummary();
}

void GameStatusTab::onDemoModeToggled(bool enabled)
{
    if (enabled) {
        startDemo();
    } else {
        stopDemo();
    }

    updateTelemetry();
}

void GameStatusTab::onDemoTick()
{
    if (!ui->checkDemoMode->isChecked()) {
        return;
    }

    m_timeToNextScanMs -= m_demoTimer->interval();
    if (m_timeToNextScanMs > 0) {
        updateTelemetry();
        return;
    }

    m_lastScan = QDateTime::currentDateTime();
    m_timeToNextScanMs = ui->spinScanInterval->value();

    const QStringList pool = demoGamePool();
    const bool running = QRandomGenerator::global()->bounded(100) >= 30;

    if (running && !pool.isEmpty()) {
        const int index = QRandomGenerator::global()->bounded(pool.size());
        m_currentGame = pool.at(index);
        appendScanLog(tr("Detected %1").arg(m_currentGame));
    } else {
        m_currentGame.clear();
        appendScanLog(tr("No game detected"));
    }

    setRunState(running);
    updateTelemetry();
}

void GameStatusTab::onScanIntervalChanged(int value)
{
    m_timeToNextScanMs = qMax(500, value);
    updateTelemetry();
}

void GameStatusTab::onSceneMappingChanged()
{
    updateMappingSummary();
}

void GameStatusTab::startDemo()
{
    if (m_demoTimer->isActive()) {
        return;
    }

    m_timeToNextScanMs = ui->spinScanInterval->value();
    setRunState(false);
    m_demoTimer->start();
    ui->labelHeaderStatus->setText(tr("Scan: Live"));
}

void GameStatusTab::stopDemo()
{
    m_demoTimer->stop();
    setRunState(false);
    m_currentGame.clear();
    m_timeToNextScanMs = ui->spinScanInterval->value();
    ui->labelHeaderStatus->setText(tr("Scan: Idle"));
}

void GameStatusTab::updateTelemetry()
{
    const QString nextScanText = tr("%1 s").arg(qMax(0, m_timeToNextScanMs) / 1000);
    ui->labelNextScanValue->setText(nextScanText);

    if (m_lastScan.isValid()) {
        ui->labelLastScanValue->setText(m_lastScan.toString("HH:mm:ss"));
    } else {
        ui->labelLastScanValue->setText(tr("Never"));
    }

    ui->labelCurrentGameValue->setText(m_currentGame.isEmpty() ? tr("None") : m_currentGame);
    ui->labelHeaderStatus->setText(ui->checkDemoMode->isChecked() ? tr("Scan: Live") : tr("Scan: Idle"));
}

void GameStatusTab::appendScanLog(const QString &message)
{
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    ui->listScanLog->insertItem(0, QString("[%1] %2").arg(timestamp, message));

    const int maxItems = 12;
    while (ui->listScanLog->count() > maxItems) {
        delete ui->listScanLog->takeItem(ui->listScanLog->count() - 1);
    }
}

void GameStatusTab::updateMappingSummary()
{
    const QStringList games = getMonitoredGames();
    const QString runningScene = ui->comboSceneGameRunning->currentText();
    const QString idleScene = ui->comboSceneNoGame->currentText();

    const int rows = games.size() + 1;
    ui->tableMappingSummary->setRowCount(rows);

    int row = 0;
    for (const QString &game : games) {
        auto *gameItem = new QTableWidgetItem(game);
        auto *sceneItem = new QTableWidgetItem(runningScene.isEmpty() ? tr("Unassigned") : runningScene);
        ui->tableMappingSummary->setItem(row, 0, gameItem);
        ui->tableMappingSummary->setItem(row, 1, sceneItem);
        ++row;
    }

    auto *idleItem = new QTableWidgetItem(tr("No game running"));
    auto *idleSceneItem = new QTableWidgetItem(idleScene.isEmpty() ? tr("Unassigned") : idleScene);
    ui->tableMappingSummary->setItem(row, 0, idleItem);
    ui->tableMappingSummary->setItem(row, 1, idleSceneItem);
}

QStringList GameStatusTab::demoGamePool() const
{
    QStringList games = getMonitoredGames();
    if (!games.isEmpty()) {
        return games;
    }

    return {
        QStringLiteral("valorant.exe"),
        QStringLiteral("cs2.exe"),
        QStringLiteral("eldenring.exe"),
        QStringLiteral("fortnite.exe"),
        QStringLiteral("apexlegends.exe")
    };
}

void GameStatusTab::setRunState(bool running)
{
    m_isRunning = running;

    if (running) {
        ui->labelRunStateBadge->setText(tr("RUNNING"));
        ui->labelRunStateBadge->setStyleSheet(
            "background-color: #00f5d4; color: #06211d; border: 1px solid #00c3b0;"
            "border-radius: 10px; padding: 4px 10px; font-weight: 600;");
    } else {
        ui->labelRunStateBadge->setText(tr("IDLE"));
        ui->labelRunStateBadge->setStyleSheet(
            "background-color: #2a2f3a; color: #d6e4ff; border: 1px solid #3a4a68;"
            "border-radius: 10px; padding: 4px 10px; font-weight: 600;");
    }
}
