#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QString>
#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QTimer>
#include <QTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <vector>
#include <map>
#include <set>
#include "synchronizer.h" 

class SynchronizationMechanism;

class SynchronizationSimulatorWidget : public QWidget {
    Q_OBJECT
public:
    explicit SynchronizationSimulatorWidget(QStackedWidget* mainStack, QWidget* menuWidget, QWidget *parent = nullptr);
    ~SynchronizationSimulatorWidget();

signals:
    void backToMenuRequested();

private slots:
    void onSyncTypeChanged();
    void nextAnimationStep();

private:
    void setupUI();
    void loadProcessesFromDialog();
    void loadResourcesFromDialog();
    void loadActionsFromDialog();
    void runSynchronization(const QString &mechanism);
    void showSimulationEvents(const std::vector<SyncEvent>& events);
    void setupEmptyTimeline();
    void clearAll();
    void showInfo();
    void updateInfoDisplay();
    QPushButton* createButton(const QString &text, const QString &color);

    // UI Components
    QStackedWidget* mainStack_;
    QWidget* menuWidget_;
    QComboBox* syncTypeCombo;
    QLabel* statusLabel;
    QLabel* cycleLabel;
    QTextEdit* infoDisplay;
    QTableWidget *syncTable;
    QTableWidget *processTable;
    QTableWidget *resourceTable;
    QTableWidget *actionTable;
    QWidget* simulationArea;
    
    // Animation
    QTimer* animationTimer;
    int currentAnimationCycle;
    int maxCycles;
    std::vector<SyncEvent> currentEvents;
    
    // Data
    std::vector<Resource> resources;
    std::vector<Action> actions;
    std::vector<Process> processes;
    SynchronizationMechanism *syncMechanism;
    QString currentSyncType;
    
    // Colors
    std::map<QString, QColor> processColors;
    QStringList colorPalette = {"#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4", "#FFEAA7", "#DDA0DD", "#98D8C8", "#F7DC6F"};

    // Buttons
    QPushButton *loadResBtn;
    QPushButton *loadActBtn;
};