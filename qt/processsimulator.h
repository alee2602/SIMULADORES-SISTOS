#ifndef PROCESS_SIMULATOR_H
#define PROCESS_SIMULATOR_H

#include <QWidget>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <vector>
#include <QString>

#include "scheduler.h"
#include "GanttChartWidget.h"
#include "synchronizer.h"
#include "loader.h"
#include "utils.h"

class ProcessSimulator : public QWidget {
    Q_OBJECT

public:
    ProcessSimulator(QWidget* parent = nullptr);
    ~ProcessSimulator();

private:
     QScrollArea *ganttScrollArea; 
     void displayAlgorithmResultInList(const QString& title, 
                                 const std::vector<ExecutionSlice>& timeline, 
                                 const std::vector<Process>& processResults);

    struct AlgorithmConfig {
        QString name;
        int quantum;
        int aging;
    };

    std::vector<AlgorithmConfig> selectedAlgorithms;
    std::vector<Process> originalProcesses;

    QWidget *multiSelectionWidget;
    QWidget *sequentialSimWidget;

    QSpinBox *quantumSpinBox;
    QSpinBox *agingSpinBox;

    QLabel *simTitleLabel;
    QLabel *metricsLabel;

QTimer *simulationTimer;
    void setupAlgorithmSelection(QVBoxLayout* layout);
    void runSelectedAlgorithms();
    void displayAlgorithmResult(const QString& title, const std::vector<ExecutionSlice>& timeline);

    QCheckBox *fifoCheck, *sjfCheck, *srtfCheck, *rrCheck, *priorityCheck;
    QCheckBox* createStyledCheckBox(const QString &text, const QString &color);
    void setupTableWithScroll(QTableWidget* table);
    QCheckBox *fifoCheckMulti, *sjfCheckMulti, *srtfCheckMulti, *rrCheckMulti, *priorityCheckMulti;
    QCheckBox *agingEnabledCheck;
    QVBoxLayout *resultsLayout;
    QWidget *resultsArea;

    // Data
    std::vector<Process> processes;
    std::vector<Resource> resources;
    std::vector<Action> actions;
    SynchronizationMechanism* syncMechanism;

    // UI components
    QStackedWidget* mainStack;
    QWidget* menuWidget;
    QWidget* schedulingWidget;
    QWidget* synchronizationWidget;

    GanttChartWidget* mainGanttChart;        // Para la pantalla principal de scheduling
    GanttChartWidget* sequentialGanttChart;  // Para la simulación secuencial

    QTableWidget* processTable;
    QTableWidget* metricsTable;
    QTableWidget* syncTable;
    QLabel* statusLabel;

    void setupMultiSelectionWidget();
    void setupSequentialSimWidget();
    void simulateNextAlgorithm();

    // Setup methods
    void setupUI();
    void setupMenuWidget();
    void setupSchedulingWidget();
    void setupSynchronizationWidget();

    // Button styles
    QPushButton* createMenuButton(const QString& text, const QString& color, const QString& description);
    QPushButton* createButton(const QString& text, const QString& color);

    // Utility
    QString lightenColor(const QString& color);
    QString darkenColor(const QString& color);

    // Process management
    void loadProcessesFromDialog();
    void generateSampleProcesses();
    void generateSampleResources();
    void generateSampleActions();
    void updateProcessTable();
    void updateMetricsTable();

    // Scheduling
    void runFIFO();
    void runSJF();
    void runSRTF();
    void runRoundRobin();
    void runPriority();

    // Synchronization
    void runSynchronization(const QString& mechanism);
    void startMainAnimation();
};

#endif
