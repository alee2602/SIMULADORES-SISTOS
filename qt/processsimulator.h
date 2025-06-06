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
    explicit ProcessSimulator(QStackedWidget* mainStack, QWidget* menuWidget, QWidget *parent = nullptr);
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

    QLabel *simTitleLabel;
    QLabel *metricsLabel;

    QTimer *simulationTimer;
    void setupAlgorithmSelection(QVBoxLayout* layout);
    void runSelectedAlgorithms();
    void displayAlgorithmResult(const QString& title, const std::vector<ExecutionSlice>& timeline);

    QCheckBox *fifoCheck, *sjfCheck, *srtfCheck, *rrCheck, *priorityCheck;
    QCheckBox* createStyledCheckBox(const QString &text, const QString &color);
    void setupTableWithScroll(QTableWidget* table);
    void setupTableStyle(QTableWidget* table);
    QCheckBox *fifoCheckMulti, *sjfCheckMulti, *srtfCheckMulti, *rrCheckMulti, *priorityCheckMulti;
    QCheckBox *agingEnabledCheck;
    QVBoxLayout *resultsLayout;
    QWidget *resultsArea;
    QLabel *quantumLabel;
    QSpinBox *quantumSpinBox;

    QLabel *agingLabel;
    QSpinBox *agingSpinBox;

    QLabel* metricsLabelBelowGantt = nullptr;

    // Data
    std::vector<Process> processes;
    std::vector<Resource> resources;
    std::vector<Action> actions;
    SynchronizationMechanism* syncMechanism;

    // UI components
    QStackedWidget* mainStack;
    QWidget* schedulingWidget;
    QWidget* synchronizationWidget;

    GanttChartWidget* mainGanttChart;     
    GanttChartWidget* sequentialGanttChart;  

    QTableWidget* processTable;
    QTableWidget* metricsTable;
    QTableWidget* syncTable;
    QLabel* statusLabel;

    void setupMultiSelectionWidget();
    void setupSequentialSimWidget();
    void simulateNextAlgorithm();

    // Setup methods
    void setupUI(QWidget* menuWidget);
    void setupSchedulingWidget(QWidget* menuWidget);
    void setupSequentialSimWidget(QWidget* menuWidget);

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
    void updateProcessTable();
    void updateMetricsTable();

    // Scheduling
    void runFIFO();
    void runSJF();
    void runSRTF();
    void runRoundRobin();
    void runPriority();

    // Synchronization
    void startMainAnimation();

    std::vector<std::vector<ExecutionSlice>> algorithmTimelines;
    QStringList algorithmNames;
public:
    void runSelectedAlgorithmsComparison();
    void displayComparisonResult(const QStringList& algorithms, const std::vector<std::vector<ExecutionSlice>>& timelines);

private slots:
    void runNextAlgorithmInSequence();
    void cleanProcesses(); 
    void displayComparisonTableOnly(const QStringList& algorithms, 
                                const std::vector<double>& waitingTimes,
                                const std::vector<double>& turnaroundTimes);
                                
signals:
    void returnToMenuRequested();

private:
    QStringList selectedAlgorithmsForSequential;
    int currentAlgorithmIndex;

    // Estructura para almacenar resultados de simulación
    struct SimulationResult {
        QString algorithmName;
        double avgWaitingTime;
        double avgTurnaroundTime;
        std::vector<ExecutionSlice> timeline;
        std::vector<Process> processResults;
    };

    std::vector<SimulationResult> sequentialResults;
    QWidget* resultsWindow;
    QVBoxLayout* resultsWindowLayout;
    void showSimulationSummary(QWidget* menuWidget);
    void createComparisonTable();
    void createIndividualResultWidget(const SimulationResult& result, QVBoxLayout* layout);
    int selectedQuantum = 2;
    bool agingEnabled = false;
    int selectedAging = 5;

    QWidget* menuWidget_;
};

#endif
