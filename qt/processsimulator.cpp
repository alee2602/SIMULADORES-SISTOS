#include "processsimulator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextEdit>
#include <QHeaderView>
#include <QFont>
#include <QSpinBox>
#include <QColor>
#include "ganttchartwidget.h"
#include "loader.h"

ProcessSimulator::ProcessSimulator(QWidget *parent)
    : QWidget(parent), syncMechanism(nullptr)
{
    setupUI();
    generateSampleProcesses();
    generateSampleResources();
    generateSampleActions();
    generateSampleProcesses();
    originalProcesses = processes;
}

ProcessSimulator::~ProcessSimulator()
{
    delete syncMechanism;
}

QString ProcessSimulator::lightenColor(const QString &color)
{
    QColor c(color);
    return c.lighter(120).name();
}

QString ProcessSimulator::darkenColor(const QString &color)
{
    QColor c(color);
    return c.darker(120).name();
}

QPushButton *ProcessSimulator::createMenuButton(const QString &text, const QString &color, const QString &description)
{
    QPushButton *btn = new QPushButton(text);
    btn->setMinimumSize(900, 140);
    btn->setMaximumSize(1200, 140);
    btn->setStyleSheet(QString("QPushButton { background-color: %1; color: black; font-size: 18px; border-radius: 20px; padding: 20px; text-align: left; }"
                            "QPushButton:hover { background-color: %2; }")
                        .arg(color)
                        .arg(lightenColor(color)));
    btn->setToolTip(description);
    return btn;
}

QPushButton *ProcessSimulator::createButton(const QString &text, const QString &color)
{
    QPushButton *btn = new QPushButton(text);
    btn->setMinimumSize(140, 45);
    btn->setStyleSheet(QString("QPushButton { background-color: %1; color: white; font-weight: bold; border-radius: 10px; padding: 10px; }"
                            "QPushButton:hover { background-color: %2; }")
                        .arg(color)
                        .arg(darkenColor(color)));
    return btn;
}

void ProcessSimulator::generateSampleProcesses()
{
    QString path = "data/processes_5.txt";
    if (!QFile::exists(path))
    {
        QMessageBox::warning(this, "Error", QString("File not found: %1").arg(path));
        return;
    }
    processes = loadProcesses(path);
    originalProcesses = processes; 
    updateProcessTable();
    statusLabel->setText(QString("Loaded %1 processes from %2").arg(processes.size()).arg(path));
}

void ProcessSimulator::generateSampleResources()
{
    QString path = "data/resources.txt";
    if (!QFile::exists(path))
    {
        QMessageBox::warning(this, "Error", QString("File not found: %1").arg(path));
        return;
    }
    resources = loadResources(path);
}

void ProcessSimulator::generateSampleActions()
{
    QString path = "data/actions.txt";
    if (!QFile::exists(path))
    {
        QMessageBox::warning(this, "Error", QString("File not found: %1").arg(path));
        return;
    }
    actions = loadActions(path);
}

void ProcessSimulator::setupAlgorithmSelection(QVBoxLayout* layout) {
    QLabel* instruction = new QLabel("Selecciona los algoritmos a simular:");
    instruction->setFont(QFont("Arial", 14));
    layout->addWidget(instruction);

    QHBoxLayout* checkboxLayout = new QHBoxLayout();

    fifoCheck = createStyledCheckBox("FIFO", "#FFE4E1");       
    sjfCheck = createStyledCheckBox("SJF", "#E0F6FF");         
    srtfCheck = createStyledCheckBox("SRTF", "#F0FFF0");       
    rrCheck = createStyledCheckBox("Round Robin", "#FFF8DC");   
    priorityCheck = createStyledCheckBox("Priority", "#F0E68C");

    checkboxLayout->addWidget(fifoCheck);
    checkboxLayout->addWidget(sjfCheck);
    checkboxLayout->addWidget(srtfCheck);
    checkboxLayout->addWidget(rrCheck);
    checkboxLayout->addWidget(priorityCheck);

    layout->addLayout(checkboxLayout);

    QPushButton* runAllBtn = createButton("Simular Algoritmos Seleccionados", "#28a745");
    QPushButton* compareBtn = createButton("Comparar Algoritmos", "#17a2b8");
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(runAllBtn);
    buttonLayout->addWidget(compareBtn);
    layout->addLayout(buttonLayout);

    resultsArea = new QWidget();
    resultsLayout = new QVBoxLayout(resultsArea);
    layout->addWidget(resultsArea);

    connect(runAllBtn, &QPushButton::clicked, this, &ProcessSimulator::runSelectedAlgorithms);
    connect(compareBtn, &QPushButton::clicked, this, &ProcessSimulator::runSelectedAlgorithmsComparison);
}

void ProcessSimulator::runSelectedAlgorithms() {
    QLayoutItem* item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    std::vector<Process> originalProcessesCopy = originalProcesses;
    QStringList selectedAlgorithmNames;
    std::vector<std::vector<ExecutionSlice>> algorithmTimelines;
    int laneIndex = 0;

    if (fifoCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runFIFO(processes);
        for (auto& slice : timeline) {
            slice.algorithm = "FIFO";
            slice.lane = laneIndex;
        }
        algorithmTimelines.push_back(timeline);
        selectedAlgorithmNames.append("FIFO");
        laneIndex++;
    }
    if (sjfCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runSJF(processes);
        for (auto& slice : timeline) {
            slice.algorithm = "SJF";
            slice.lane = laneIndex;
        }
        algorithmTimelines.push_back(timeline);
        selectedAlgorithmNames.append("SJF");
        laneIndex++;
    }
    if (srtfCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runSRT(processes);
        for (auto& slice : timeline) {
            slice.algorithm = "SRTF";
            slice.lane = laneIndex;
        }
        algorithmTimelines.push_back(timeline);
        selectedAlgorithmNames.append("SRTF");
        laneIndex++;
    }
    if (rrCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runRoundRobin(processes, 2);
        for (auto& slice : timeline) {
            slice.algorithm = "Round Robin";
            slice.lane = laneIndex;
        }
        algorithmTimelines.push_back(timeline);
        selectedAlgorithmNames.append("Round Robin");
        laneIndex++;
    }
    if (priorityCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runPriority(processes, true);
        for (auto& slice : timeline) {
            slice.algorithm = "Priority";
            slice.lane = laneIndex;
        }
        algorithmTimelines.push_back(timeline);
        selectedAlgorithmNames.append("Priority");
        laneIndex++;
    }

    // Mostrar todos los algoritmos seleccionados en el Gantt principal (mainGanttChart)
    if (!algorithmTimelines.empty() && mainGanttChart) {
        std::vector<ExecutionSlice> combinedTimeline;
        for (const auto& timeline : algorithmTimelines) {
            combinedTimeline.insert(combinedTimeline.end(), timeline.begin(), timeline.end());
        }
        mainGanttChart->setComparisonMode(true);
        mainGanttChart->setAlgorithmNames(selectedAlgorithmNames);
        mainGanttChart->setTimeline(combinedTimeline);

        // Calcular promedios y mostrar título
        QString title = "Comparación: " + selectedAlgorithmNames.join(", ");
        double totalWaiting = 0, totalTurnaround = 0;
        int totalCount = 0;
        for (int i = 0; i < selectedAlgorithmNames.size(); ++i) {
            processes = originalProcessesCopy;
            if (selectedAlgorithmNames[i] == "FIFO") SchedulingAlgorithms::runFIFO(processes);
            else if (selectedAlgorithmNames[i] == "SJF") SchedulingAlgorithms::runSJF(processes);
            else if (selectedAlgorithmNames[i] == "SRTF") SchedulingAlgorithms::runSRT(processes);
            else if (selectedAlgorithmNames[i] == "Round Robin") SchedulingAlgorithms::runRoundRobin(processes, 2);
            else if (selectedAlgorithmNames[i] == "Priority") SchedulingAlgorithms::runPriority(processes, true);
            for (const auto& p : processes) {
                totalWaiting += p.waiting_time;
                totalTurnaround += (p.finish_time - p.arrival_time);
                totalCount++;
            }
        }
        double avgWaiting = totalCount ? totalWaiting / totalCount : 0;
        double avgTurnaround = totalCount ? totalTurnaround / totalCount : 0;
        statusLabel->setText(title + QString(" | Avg Waiting: %1 | Avg Completion: %2")
            .arg(avgWaiting, 0, 'f', 2).arg(avgTurnaround, 0, 'f', 2));
    } else if (mainGanttChart) {
        mainGanttChart->setTimeline(std::vector<ExecutionSlice>());
        statusLabel->setText("Selecciona al menos un algoritmo para ver resultados en el chart principal");
    }

    processes = originalProcessesCopy;
    updateProcessTable();
}

// Nueva función para mostrar resultados sin limpiar todo
void ProcessSimulator::displayAlgorithmResultInList(const QString& title, 
                                                   const std::vector<ExecutionSlice>& timeline, 
                                                   const std::vector<Process>& processResults) {
    QLabel* header = new QLabel(title);
    header->setFont(QFont("Arial", 16, QFont::Bold));
    header->setStyleSheet("color: #2c3e50; margin-top: 10px;");
    resultsLayout->addWidget(header);

    // Crear un nuevo chart para mostrar en los resultados
    GanttChartWidget* chart = new GanttChartWidget();
    chart->setTimeline(timeline);
    chart->setFixedHeight(150);
    resultsLayout->addWidget(chart);

    // Calcular y mostrar métricas
    double totalWaiting = 0;
    double totalTurnaround = 0;
    for (const auto& process : processResults) {
        totalWaiting += process.waiting_time;
        totalTurnaround += (process.finish_time - process.arrival_time);
    }

    double avgWaiting = processResults.empty() ? 0 : totalWaiting / processResults.size();
    double avgTurnaround = processResults.empty() ? 0 : totalTurnaround / processResults.size();

    QLabel* summary = new QLabel(QString("Avg Waiting Time: %1 | Avg Completion Time: %2")
                                .arg(avgWaiting, 0, 'f', 2)
                                .arg(avgTurnaround, 0, 'f', 2));
    summary->setFont(QFont("Arial", 12));
    summary->setStyleSheet("color: #6c757d; margin-bottom: 20px;");
    resultsLayout->addWidget(summary);
    
    // Agregar separador visual
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: #e9ecef; margin: 10px 0;");
    resultsLayout->addWidget(line);
}


void ProcessSimulator::displayAlgorithmResult(const QString& title, const std::vector<ExecutionSlice>& timeline) {
    mainGanttChart->setTimeline(timeline);
    double totalWaiting = 0;
    double totalTurnaround = 0;
    for (const auto& process : processes) {
        totalWaiting += process.waiting_time;
        totalTurnaround += (process.finish_time - process.arrival_time);
    }

    double avgWaiting = processes.empty() ? 0 : totalWaiting / processes.size();
    double avgTurnaround = processes.empty() ? 0 : totalTurnaround / processes.size();

    updateProcessTable();
    updateMetricsTable();

    statusLabel->setText(QString("Average Waiting Time: %1 | Average Turnaround Time: %2")
                        .arg(avgWaiting, 0, 'f', 2)
                        .arg(avgTurnaround, 0, 'f', 2));
}

void ProcessSimulator::updateProcessTable()
{
    processTable->setRowCount(processes.size());
    for (int i = 0; i < processes.size(); ++i)
    {
        processTable->setItem(i, 0, new QTableWidgetItem(processes[i].pid));
        processTable->setItem(i, 1, new QTableWidgetItem(QString::number(processes[i].burst_time)));
        processTable->setItem(i, 2, new QTableWidgetItem(QString::number(processes[i].arrival_time)));
        processTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].priority)));
        QTableWidgetItem *colorItem = new QTableWidgetItem("");
        colorItem->setBackground(processes[i].color);
        processTable->setItem(i, 4, colorItem);
    }
}

void ProcessSimulator::updateMetricsTable()
{
    metricsTable->setRowCount(processes.size());
    double totalWaiting = 0;
    double totalTurnaround = 0;
    for (int i = 0; i < processes.size(); ++i)
    {
        metricsTable->setItem(i, 0, new QTableWidgetItem(processes[i].pid));
        metricsTable->setItem(i, 1, new QTableWidgetItem(QString::number(processes[i].start_time)));
        metricsTable->setItem(i, 2, new QTableWidgetItem(QString::number(processes[i].finish_time)));
        metricsTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].waiting_time)));
        totalWaiting += processes[i].waiting_time;
        totalTurnaround += (processes[i].finish_time - processes[i].arrival_time);
    }
    double avgWaiting = totalWaiting / processes.size();
    double avgTurnaround = totalTurnaround / processes.size();
    statusLabel->setText(QString("Average Waiting Time: %1 | Average Turnaround Time: %2").arg(avgWaiting, 0, 'f', 2).arg(avgTurnaround, 0, 'f', 2));
}

void ProcessSimulator::loadProcessesFromDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Processes", "", "Text Files (*.txt)");
    if (fileName.isEmpty())
        return;

    processes = loadProcesses(fileName);
    originalProcesses = processes; 
    updateProcessTable();
    statusLabel->setText(QString("Loaded %1 processes from file").arg(processes.size()));

    QMessageBox::information(this, "Archivo cargado", QString("Se cargaron %1 procesos desde %2").arg(processes.size()).arg(fileName));
}

void ProcessSimulator::runSynchronization(const QString &mechanism)
{
    if (processes.empty() || resources.empty() || actions.empty())
    {
        QMessageBox::warning(this, "Warning", "No processes, resources, or actions loaded!");
        return;
    }
    delete syncMechanism;
    if (mechanism == "Mutex Lock")
    {
        syncMechanism = new MutexLock(resources);
    }
    else if (mechanism == "Semaphore")
    {
        syncMechanism = new Semaphore(resources);
    }
    auto events = SynchronizationSimulator::simulateSynchronization(processes, resources, actions, syncMechanism);
    syncTable->setRowCount(events.size());
    QString resultText = QString("=== %1 Simulation Results ===\n").arg(mechanism);
    for (int i = 0; i < events.size(); ++i)
    {
        const auto &event = events[i];
        syncTable->setItem(i, 0, new QTableWidgetItem(event.pid));
        syncTable->setItem(i, 1, new QTableWidgetItem(event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING"));
        syncTable->setItem(i, 2, new QTableWidgetItem(event.resource));
        syncTable->setItem(i, 3, new QTableWidgetItem(event.action_type));
        syncTable->setItem(i, 4, new QTableWidgetItem(QString::number(event.cycle)));
        resultText += QString("Cycle %1: Process %2 %3 %4 (%5)\n").arg(event.cycle).arg(event.pid).arg(event.action_type).arg(event.resource).arg(event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING");
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle("Synchronization Results");
    msgBox.setText(QString("%1 simulation completed successfully!").arg(mechanism));
    msgBox.setDetailedText(resultText);
    msgBox.exec();
}

void ProcessSimulator::setupUI()
{
    setWindowTitle("Advanced Process Scheduling & Synchronization Simulator");
    setFixedSize(1400, 1000);
    setStyleSheet("background-color: #f8f9fa;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainStack = new QStackedWidget();

    setupMenuWidget();
    setupSchedulingWidget();
    setupSynchronizationWidget();
    setupMultiSelectionWidget();
    setupSequentialSimWidget();


    mainLayout->addWidget(mainStack);
}

void ProcessSimulator::setupMenuWidget()
{
    menuWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(menuWidget);
    layout->setSpacing(20);
    layout->setContentsMargins(50, 30, 50, 30);

    QLabel *title = new QLabel("PROCESS SIMULATOR");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Arial", 32, QFont::Bold));
    title->setStyleSheet("color: black; background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 rgb(168, 220, 230), stop: 1 rgb(184, 231, 255)); border-radius: 20px; padding: 30px; margin: 20px;");

    QLabel *subtitle = new QLabel("Choose Simulation Type");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setFont(QFont("Arial", 18));
    subtitle->setStyleSheet("color: #6c757d; margin: 10px;");

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(25);
    buttonLayout->setAlignment(Qt::AlignCenter);

    QPushButton *schedulingBtn = createMenuButton("Simulador de Algoritmos de Calendarización", "#c4dafa", "Simulate process scheduling algorithms like FIFO, SJF, Round Robin, and Priority");
    QPushButton *syncBtn = createMenuButton("Simulador de Mecanismos de Sincronización", "#c4e5fb", "Simulate synchronization mechanisms like Mutex Locks and Semaphores");
    QPushButton *multiSimBtn = createMenuButton("Simulación Secuencial (con Checkboxes)", "#c2f0c2", "Selecciona múltiples algoritmos y simúlalos uno por uno");
buttonLayout->addWidget(multiSimBtn);

connect(multiSimBtn, &QPushButton::clicked, [this]() {
    mainStack->setCurrentWidget(multiSelectionWidget);
});

    buttonLayout->addWidget(schedulingBtn);
    buttonLayout->addWidget(syncBtn);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addStretch(1);
    layout->addLayout(buttonLayout);
    layout->addStretch(2);

    connect(schedulingBtn, &QPushButton::clicked, [this]()
            { mainStack->setCurrentWidget(schedulingWidget); });
    connect(syncBtn, &QPushButton::clicked, [this]()
            { mainStack->setCurrentWidget(synchronizationWidget); });

    mainStack->addWidget(menuWidget);
}

void ProcessSimulator::setupMultiSelectionWidget() {
    multiSelectionWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(multiSelectionWidget);

    QLabel *title = new QLabel("Selecciona los algoritmos que deseas simular:");
    title->setFont(QFont("Arial", 16, QFont::Bold));
    layout->addWidget(title);

    // Checkboxes de algoritmos
    fifoCheckMulti = createStyledCheckBox("FIFO", "#FFE4E1");        // Rosa pastel
    sjfCheckMulti = createStyledCheckBox("SJF", "#E0F6FF");          // Azul pastel
    srtfCheckMulti = createStyledCheckBox("SRTF", "#F0FFF0");        // Verde pastel
    rrCheckMulti = createStyledCheckBox("Round Robin", "#FFF8DC");    // Amarillo pastel
    priorityCheckMulti = createStyledCheckBox("Priority", "#F0E68C"); // Dorado pastel

    QHBoxLayout *checksLayout = new QHBoxLayout();
    checksLayout->addWidget(fifoCheckMulti);
    checksLayout->addWidget(sjfCheckMulti);
    checksLayout->addWidget(srtfCheckMulti);
    checksLayout->addWidget(rrCheckMulti);
    checksLayout->addWidget(priorityCheckMulti);
    layout->addLayout(checksLayout);

    // Parámetros: Quantum y Aging
    QHBoxLayout *paramLayout = new QHBoxLayout();

    QLabel *quantumLabel = new QLabel("Quantum:");
    quantumSpinBox = new QSpinBox();
    quantumSpinBox->setRange(1, 10);
    quantumSpinBox->setValue(2);

    QLabel *agingLabel = new QLabel("Aging:");
    agingSpinBox = new QSpinBox();
    agingSpinBox->setRange(1, 10);
    agingSpinBox->setValue(5);

    agingEnabledCheck = createStyledCheckBox("Habilitar envejecimiento (Priority)", "#E6E6FA"); // Lavanda pastel
    agingSpinBox->setEnabled(false);

    connect(agingEnabledCheck, &QCheckBox::toggled, agingSpinBox, &QWidget::setEnabled);

    paramLayout->addWidget(quantumLabel);
    paramLayout->addWidget(quantumSpinBox);
    paramLayout->addSpacing(20);
    paramLayout->addWidget(agingLabel);
    paramLayout->addWidget(agingSpinBox);
    layout->addLayout(paramLayout);
    layout->addWidget(agingEnabledCheck);

    // Botón para cargar procesos
    QPushButton *loadBtn = createButton("Cargar Procesos", "#5a68a5");
    connect(loadBtn, &QPushButton::clicked, this, [this]() {
        loadProcessesFromDialog();
        originalProcesses = processes;
        QMessageBox::information(this, "Carga Exitosa", "Procesos cargados correctamente.");
    });
    layout->addWidget(loadBtn);

    // Botón para iniciar simulación
    QPushButton *startBtn = createButton("Iniciar Simulación", "#28a745");
    layout->addWidget(startBtn);

    connect(startBtn, &QPushButton::clicked, [this]() {
        selectedAlgorithms.clear();

        if (fifoCheckMulti->isChecked()) selectedAlgorithms.push_back({"FIFO", 0, 0});
        if (sjfCheckMulti->isChecked()) selectedAlgorithms.push_back({"SJF", 0, 0});
        if (srtfCheckMulti->isChecked()) selectedAlgorithms.push_back({"SRTF", 0, 0});
        if (rrCheckMulti->isChecked()) selectedAlgorithms.push_back({"RR", quantumSpinBox->value(), 0});
        if (priorityCheckMulti->isChecked()) {
            int agingVal = agingEnabledCheck->isChecked() ? agingSpinBox->value() : 0;
            selectedAlgorithms.push_back({"PRIORITY", 0, 0});
        }

        processes = originalProcesses;
        mainStack->setCurrentWidget(sequentialSimWidget);
        simulateNextAlgorithm();
    });

    mainStack->addWidget(multiSelectionWidget);
}

void ProcessSimulator::setupSchedulingWidget()
{
    schedulingWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(schedulingWidget);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backBtn = createButton("← Back to Menu", "#6c757d");
    QLabel *title = new QLabel("PROCESS SCHEDULING ALGORITHMS");
    title->setFont(QFont("Arial", 20, QFont::Bold));
    title->setStyleSheet("color: #2c3e50;");
    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(title, 1, Qt::AlignCenter);
    headerLayout->addStretch();

    QHBoxLayout *processLayout = new QHBoxLayout();
    QPushButton *loadBtn = createButton("Load Processes", "#6c85bd");
    QPushButton *generateBtn = createButton("Generate Sample", "#70a1a8");
    processLayout->addWidget(loadBtn);
    processLayout->addWidget(generateBtn);
    processLayout->addStretch();

    mainGanttChart = new GanttChartWidget();  
    QScrollArea *mainGanttScrollArea = mainGanttChart->createScrollArea();

    QHBoxLayout *animLayout = new QHBoxLayout();
    QPushButton *startBtn = createButton("▶ Start Animation", "#30c752");
    QPushButton *stopBtn = createButton("Stop Animation", "#dc3545");
    QSpinBox *speedSpinBox = new QSpinBox();
    speedSpinBox->setRange(100, 2000);
    speedSpinBox->setValue(500);
    speedSpinBox->setSuffix(" ms");
    animLayout->addWidget(new QLabel("Speed:"));
    animLayout->addWidget(speedSpinBox);
    animLayout->addWidget(startBtn);
    animLayout->addWidget(stopBtn);
    animLayout->addStretch();

    QHBoxLayout *tablesLayout = new QHBoxLayout();
    processTable = new QTableWidget();
    processTable->setColumnCount(5);
    processTable->setHorizontalHeaderLabels({"PID", "Burst Time", "Arrival Time", "Priority", "Color"});
    processTable->horizontalHeader()->setStretchLastSection(true);
    processTable->setStyleSheet("background-color: white; border: 1px solid #ddd;");

    metricsTable = new QTableWidget();
    metricsTable->setColumnCount(4);
    metricsTable->setHorizontalHeaderLabels({"PID", "Start Time", "Finish Time", "Waiting Time"});
    metricsTable->horizontalHeader()->setStretchLastSection(true);
    metricsTable->setStyleSheet("background-color: white; border: 1px solid #ddd;");

    tablesLayout->addWidget(processTable);
    tablesLayout->addWidget(metricsTable);

    statusLabel = new QLabel("Ready to simulate scheduling algorithms");
    statusLabel->setStyleSheet("color: #6c757d; padding: 10px; font-size: 14px;");

    layout->addLayout(headerLayout);
    layout->addLayout(processLayout);
    setupAlgorithmSelection(layout);
    layout->addWidget(mainGanttScrollArea); 
    layout->addLayout(animLayout);
    layout->addLayout(tablesLayout);
    layout->addWidget(statusLabel);

    connect(backBtn, &QPushButton::clicked, [this]() { mainStack->setCurrentWidget(menuWidget); });
    connect(loadBtn, &QPushButton::clicked, this, &ProcessSimulator::loadProcessesFromDialog);
    connect(generateBtn, &QPushButton::clicked, this, &ProcessSimulator::generateSampleProcesses);
    connect(startBtn, &QPushButton::clicked, mainGanttChart, &GanttChartWidget::startAnimation);
    connect(stopBtn, &QPushButton::clicked, mainGanttChart, &GanttChartWidget::stopAnimation);
    connect(speedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), mainGanttChart, &GanttChartWidget::setAnimationSpeed);

    mainStack->addWidget(schedulingWidget);
}


void ProcessSimulator::setupSequentialSimWidget() {
    sequentialSimWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(sequentialSimWidget);

    simTitleLabel = new QLabel("Simulando algoritmo...");
    simTitleLabel->setFont(QFont("Arial", 16, QFont::Bold));
    layout->addWidget(simTitleLabel);

    sequentialGanttChart = new GanttChartWidget();  
    QScrollArea *seqGanttScrollArea = sequentialGanttChart->createScrollArea();
    seqGanttScrollArea->setFixedHeight(220);
    layout->addWidget(seqGanttScrollArea);

    metricsLabel = new QLabel("Esperando métricas...");
    metricsLabel->setFont(QFont("Arial", 12));
    layout->addWidget(metricsLabel);

    QPushButton *returnBtn = createButton("← Volver al menú", "#6c757d");
    layout->addWidget(returnBtn);

    connect(returnBtn, &QPushButton::clicked, [this]() {
        mainStack->setCurrentWidget(menuWidget);
    });

    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &ProcessSimulator::simulateNextAlgorithm);

    mainStack->addWidget(sequentialSimWidget);
}

void ProcessSimulator::simulateNextAlgorithm() {
    if (selectedAlgorithms.empty()) {
        QTimer::singleShot(2000, this, [this]() {
            QMessageBox::information(this, "Simulación Finalizada", "Todos los algoritmos seleccionados han sido simulados.");
            mainStack->setCurrentWidget(menuWidget);
        });
        simulationTimer->stop();
        return;
    }

    AlgorithmConfig config = selectedAlgorithms.front();
    selectedAlgorithms.erase(selectedAlgorithms.begin());

    processes = originalProcesses;

    std::vector<ExecutionSlice> timeline;
    if (config.name == "FIFO") {
        timeline = SchedulingAlgorithms::runFIFO(processes);
    } else if (config.name == "SJF") {
        timeline = SchedulingAlgorithms::runSJF(processes);
    } else if (config.name == "SRTF") {
        timeline = SchedulingAlgorithms::runSRT(processes);
    } else if (config.name == "RR") {
        timeline = SchedulingAlgorithms::runRoundRobin(processes, config.quantum);
    } else if (config.name == "PRIORITY") {
        timeline = SchedulingAlgorithms::runPriority(processes, true, config.aging); 
    }

    simTitleLabel->setText("Simulando: " + config.name);


    if (sequentialGanttChart) {
        sequentialGanttChart->setTimeline(timeline);
        sequentialGanttChart->startAnimation();
    }

    double totalWait = 0, totalTurnaround = 0;
    for (const auto& p : processes) {
        totalWait += p.waiting_time;
        totalTurnaround += (p.finish_time - p.arrival_time);
    }

    metricsLabel->setText(QString("Avg Waiting Time: %1 | Avg Completion Time: %2")
        .arg(totalWait / processes.size(), 0, 'f', 2)
        .arg(totalTurnaround / processes.size(), 0, 'f', 2));

    int duration = timeline.empty() ? 2000 : (timeline.back().start_time + timeline.back().duration) * 500;
    simulationTimer->start(duration + 1000);  
}



void ProcessSimulator::setupSynchronizationWidget()
{
    synchronizationWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(synchronizationWidget);
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backBtn = createButton("← Back to Menu", "#6c757d");
    QLabel *title = new QLabel("SYNCHRONIZATION MECHANISMS");
    title->setFont(QFont("Arial", 20, QFont::Bold));
    title->setStyleSheet("color: #2c3e50;");
    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(title, 1, Qt::AlignCenter);
    headerLayout->addStretch();

    QHBoxLayout *syncLayout = new QHBoxLayout();
    syncLayout->addStretch();
    QPushButton *mutexBtn = createButton("Mutex Locks", "#e74c3c");
    QPushButton *semaphoreBtn = createButton("Semaphores", "#f39c12");
    syncLayout->addWidget(mutexBtn);
    syncLayout->addWidget(semaphoreBtn);
    syncLayout->addStretch();

    QHBoxLayout *resourceLayout = new QHBoxLayout();
    QPushButton *loadResourcesBtn = createButton("Load Resources", "#3498db");
    QPushButton *generateResourcesBtn = createButton("Generate Sample Resources", "#2ecc71");
    resourceLayout->addWidget(loadResourcesBtn);
    resourceLayout->addWidget(generateResourcesBtn);
    resourceLayout->addStretch();

    syncTable = new QTableWidget();
    syncTable->setColumnCount(5);
    syncTable->setHorizontalHeaderLabels({"PID", "State", "Resource", "Action", "Cycle"});
    syncTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    syncTable->setStyleSheet("background-color: white; border: 1px solid #ddd; font-size: 14px;");

    QTextEdit *resultsArea = new QTextEdit();
    resultsArea->setMaximumHeight(150);
    resultsArea->setStyleSheet("background-color: white; border: 1px solid #ddd;");
    resultsArea->setPlaceholderText("Synchronization results will appear here...");

    layout->addLayout(headerLayout);
    layout->addLayout(syncLayout);
    layout->addLayout(resourceLayout);
    layout->addWidget(new QLabel("Synchronization Events:"));
    layout->addWidget(syncTable);
    layout->addWidget(new QLabel("Results:"));
    layout->addWidget(resultsArea);

    connect(backBtn, &QPushButton::clicked, [this]()
            { mainStack->setCurrentWidget(menuWidget); });
    connect(mutexBtn, &QPushButton::clicked, [this]()
            { runSynchronization("Mutex Lock"); });
    connect(semaphoreBtn, &QPushButton::clicked, [this]()
            { runSynchronization("Semaphore"); });
    connect(generateResourcesBtn, &QPushButton::clicked, [this]()
            {
        generateSampleResources();
        generateSampleActions();
        QMessageBox::information(this, "Success", "Sample resources and actions generated!"); });

    mainStack->addWidget(synchronizationWidget);
}

QCheckBox* ProcessSimulator::createStyledCheckBox(const QString &text, const QString &color) {
    QCheckBox *checkbox = new QCheckBox(text);
    checkbox->setStyleSheet(QString(
        "QCheckBox {"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    padding: 8px;"
        "    border-radius: 8px;"
        "    background-color: %1;"
        "    margin: 4px;"
        "}"
        "QCheckBox:hover {"
        "    background-color: %2;"
        "}"
        "QCheckBox::indicator {"
        "    width: 20px;"
        "    height: 20px;"
        "    border-radius: 10px;"
        "    border: 2px solid #bdc3c7;"
        "    background-color: white;"
        "}"
        "QCheckBox::indicator:checked {"
        "    background-color: %3;"
        "    border: 2px solid %4;"
        "}"
    ).arg(color)
     .arg(lightenColor(color))
     .arg(darkenColor(color))
     .arg(darkenColor(color)));
    return checkbox;
}

void ProcessSimulator::startMainAnimation() {
    if (mainGanttChart && mainGanttChart->hasTimeline()) {
        mainGanttChart->startAnimation();
    } else {
        QMessageBox::information(this, "No Animation Data", 
            "Primero selecciona y simula un algoritmo para ver la animación.");
    }
}

void ProcessSimulator::runSelectedAlgorithmsComparison() {
    QLayoutItem* item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    algorithmTimelines.clear();
    algorithmNames.clear();
    std::vector<Process> originalProcessesCopy = originalProcesses;
    int laneIndex = 0;
    if (fifoCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runFIFO(processes);
        for (auto& slice : timeline) { slice.algorithm = "FIFO"; slice.lane = laneIndex; }
        algorithmTimelines.push_back(timeline);
        algorithmNames.append("FIFO");
        laneIndex++;
    }
    if (sjfCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runSJF(processes);
        for (auto& slice : timeline) { slice.algorithm = "SJF"; slice.lane = laneIndex; }
        algorithmTimelines.push_back(timeline);
        algorithmNames.append("SJF");
        laneIndex++;
    }
    if (srtfCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runSRT(processes);
        for (auto& slice : timeline) { slice.algorithm = "SRTF"; slice.lane = laneIndex; }
        algorithmTimelines.push_back(timeline);
        algorithmNames.append("SRTF");
        laneIndex++;
    }
    if (rrCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runRoundRobin(processes, 2);
        for (auto& slice : timeline) { slice.algorithm = "Round Robin"; slice.lane = laneIndex; }
        algorithmTimelines.push_back(timeline);
        algorithmNames.append("Round Robin");
        laneIndex++;
    }
    if (priorityCheck->isChecked()) {
        processes = originalProcessesCopy;
        auto timeline = SchedulingAlgorithms::runPriority(processes, true);
        for (auto& slice : timeline) { slice.algorithm = "Priority"; slice.lane = laneIndex; }
        algorithmTimelines.push_back(timeline);
        algorithmNames.append("Priority");
        laneIndex++;
    }
    if (!algorithmTimelines.empty()) {
        displayComparisonResult(algorithmNames, algorithmTimelines);
    }
    processes = originalProcessesCopy;
    updateProcessTable();
}

void ProcessSimulator::displayComparisonResult(const QStringList& algorithms, const std::vector<std::vector<ExecutionSlice>>& timelines) {
    QLabel* header = new QLabel("Comparación de Algoritmos");
    header->setFont(QFont("Arial", 18, QFont::Bold));
    header->setStyleSheet("color: #2c3e50; margin: 20px 0;");
    header->setAlignment(Qt::AlignCenter);
    resultsLayout->addWidget(header);

    std::vector<ExecutionSlice> combinedTimeline;
    for (const auto& timeline : timelines) {
        combinedTimeline.insert(combinedTimeline.end(), timeline.begin(), timeline.end());
    }

    GanttChartWidget* comparisonChart = new GanttChartWidget();
    comparisonChart->setComparisonMode(true);
    comparisonChart->setAlgorithmNames(algorithms);
    comparisonChart->setTimeline(combinedTimeline);
    comparisonChart->setFixedHeight(100 + algorithms.size() * 60);
    resultsLayout->addWidget(comparisonChart);

    QTableWidget* comparisonTable = new QTableWidget();
    comparisonTable->setRowCount(algorithms.size());
    comparisonTable->setColumnCount(3);
    comparisonTable->setHorizontalHeaderLabels({"Algoritmo", "Tiempo Promedio de Espera", "Tiempo Promedio de Retorno"});

    for (int i = 0; i < algorithms.size(); i++) {
        processes = originalProcesses;
        if (algorithms[i] == "FIFO") {
            SchedulingAlgorithms::runFIFO(processes);
        } else if (algorithms[i] == "SJF") {
            SchedulingAlgorithms::runSJF(processes);
        } else if (algorithms[i] == "SRTF") {
            SchedulingAlgorithms::runSRT(processes);
        } else if (algorithms[i] == "Round Robin") {
            SchedulingAlgorithms::runRoundRobin(processes, 2);
        } else if (algorithms[i] == "Priority") {
            SchedulingAlgorithms::runPriority(processes, true);
        }
        double totalWaiting = 0;
        double totalTurnaround = 0;
        for (const auto& process : processes) {
            totalWaiting += process.waiting_time;
            totalTurnaround += (process.finish_time - process.arrival_time);
        }
        double avgWaiting = processes.empty() ? 0 : totalWaiting / processes.size();
        double avgTurnaround = processes.empty() ? 0 : totalTurnaround / processes.size();
        comparisonTable->setItem(i, 0, new QTableWidgetItem(algorithms[i]));
        comparisonTable->setItem(i, 1, new QTableWidgetItem(QString::number(avgWaiting, 'f', 2)));
        comparisonTable->setItem(i, 2, new QTableWidgetItem(QString::number(avgTurnaround, 'f', 2)));
        QColor rowColor;
        if (algorithms[i] == "FIFO") rowColor = QColor("#FFE4E1");
        else if (algorithms[i] == "SJF") rowColor = QColor("#E0F6FF");
        else if (algorithms[i] == "SRTF") rowColor = QColor("#F0FFF0");
        else if (algorithms[i] == "Round Robin") rowColor = QColor("#FFF8DC");
        else if (algorithms[i] == "Priority") rowColor = QColor("#F0E68C");
        for (int j = 0; j < 3; j++) {
            comparisonTable->item(i, j)->setBackground(rowColor);
        }
    }
    comparisonTable->resizeColumnsToContents();
    comparisonTable->setMaximumHeight(150);
    resultsLayout->addWidget(comparisonTable);
}
