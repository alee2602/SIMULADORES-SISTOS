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

ProcessSimulator::ProcessSimulator(QStackedWidget* mainStack, QWidget* menuWidget_, QWidget *parent)
    : QWidget(parent), mainStack(mainStack), menuWidget_(menuWidget_)
{
    setupUI(menuWidget_);
    generateSampleResources();
    sequentialResults.clear(); 
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
    
    // Solo generar procesos de muestra si no hay procesos ya cargados
    if (processes.empty()) {
        processes = loadProcesses(path);
        originalProcesses = processes;
        updateProcessTable();
        statusLabel->setText(QString("Loaded %1 sample processes from %2").arg(processes.size()).arg(path));
    } else {
        // Si ya hay procesos, preguntar si se quieren reemplazar
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "Reemplazar procesos", 
            "Ya hay procesos cargados. ¿Deseas reemplazarlos con los procesos de muestra?",
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::Yes) {
            processes = loadProcesses(path);
            originalProcesses = processes;
            updateProcessTable();
            statusLabel->setText(QString("Loaded %1 sample processes from %2").arg(processes.size()).arg(path));
        }
    }
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

    // Parámetros de Quantum y Aging (visibles solo si corresponde)
    QHBoxLayout *paramLayout = new QHBoxLayout();

    QLabel *quantumLabel = new QLabel("Quantum (RR):");
    QSpinBox *quantumSpinBox = new QSpinBox();
    quantumSpinBox->setRange(1, 10);
    quantumSpinBox->setValue(2);
    quantumLabel->setVisible(false);
    quantumSpinBox->setVisible(false);

    QLabel *agingLabel = new QLabel("Aging (Priority):");
    QSpinBox *agingSpinBox = new QSpinBox();
    agingSpinBox->setRange(1, 10);
    agingSpinBox->setValue(5);
    agingLabel->setVisible(false);
    agingSpinBox->setVisible(false);

    QCheckBox *agingEnabledCheck = new QCheckBox("Habilitar envejecimiento");
    agingEnabledCheck->setVisible(false);
    agingSpinBox->setEnabled(false);

    connect(agingEnabledCheck, &QCheckBox::toggled, agingSpinBox, &QWidget::setEnabled);

    paramLayout->addWidget(quantumLabel);
    paramLayout->addWidget(quantumSpinBox);
    paramLayout->addSpacing(20);
    paramLayout->addWidget(agingLabel);
    paramLayout->addWidget(agingSpinBox);
    paramLayout->addWidget(agingEnabledCheck);
    layout->addLayout(paramLayout);

    // Mostrar/ocultar controles según selección
    connect(rrCheck, &QCheckBox::toggled, [quantumLabel, quantumSpinBox](bool checked){
        quantumLabel->setVisible(checked);
        quantumSpinBox->setVisible(checked);
    });
    connect(priorityCheck, &QCheckBox::toggled, [agingLabel, agingSpinBox, agingEnabledCheck](bool checked){
        agingLabel->setVisible(checked);
        agingSpinBox->setVisible(checked && agingEnabledCheck->isChecked());
        agingEnabledCheck->setVisible(checked);
    });
    connect(agingEnabledCheck, &QCheckBox::toggled, [agingSpinBox](bool checked){
        agingSpinBox->setVisible(checked);
    });

    QPushButton* runAllBtn = createButton("Simular Algoritmos Seleccionados", "#28a745");
    QPushButton* compareBtn = createButton("Comparar Algoritmos", "#17a2b8");
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(runAllBtn);
    buttonLayout->addWidget(compareBtn);
    layout->addLayout(buttonLayout);

    resultsArea = new QWidget();
    resultsLayout = new QVBoxLayout(resultsArea);
    layout->addWidget(resultsArea);

    // Guardar quantum y aging seleccionados al correr simulación
    connect(runAllBtn, &QPushButton::clicked, this, [this, quantumSpinBox, agingEnabledCheck, agingSpinBox]() {
        // Puedes guardar los valores en variables miembro si lo necesitas
        this->selectedQuantum = quantumSpinBox->value();
        this->agingEnabled = agingEnabledCheck->isChecked();
        this->selectedAging = agingSpinBox->value();
        runSelectedAlgorithms();
    });
    connect(compareBtn, &QPushButton::clicked, this, &ProcessSimulator::runSelectedAlgorithmsComparison);
}

void ProcessSimulator::runSelectedAlgorithms() {
    // Limpiar resultados previos
    QLayoutItem* item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Verificar que hay procesos cargados
    if (originalProcesses.empty()) {
        QMessageBox::warning(this, "No hay procesos", "Por favor carga procesos antes de simular algoritmos.");
        return;
    }

    // Preparar lista de algoritmos seleccionados
    selectedAlgorithmsForSequential.clear();
    
    if (fifoCheck->isChecked()) {
        selectedAlgorithmsForSequential.push_back("FIFO");
    }
    if (sjfCheck->isChecked()) {
        selectedAlgorithmsForSequential.push_back("SJF");
    }
    if (srtfCheck->isChecked()) {
        selectedAlgorithmsForSequential.push_back("SRTF");
    }
    if (rrCheck->isChecked()) {
        selectedAlgorithmsForSequential.push_back("Round Robin");
    }
    if (priorityCheck->isChecked()) {
        selectedAlgorithmsForSequential.push_back("Priority");
    }

    if (selectedAlgorithmsForSequential.empty()) {
        QMessageBox::warning(this, "No hay algoritmos", "Por favor selecciona al menos un algoritmo.");
        return;
    }

    // Iniciar simulación secuencial
    currentAlgorithmIndex = 0;
    runNextAlgorithmInSequence();
}

// Nueva función para ejecutar algoritmos secuencialmente
void ProcessSimulator::runNextAlgorithmInSequence() {
    if (currentAlgorithmIndex >= selectedAlgorithmsForSequential.size()) {
        statusLabel->setText("Simulación secuencial completada");
        showSimulationSummary(menuWidget_); 
        return;
    }

    QString algorithmName = selectedAlgorithmsForSequential[currentAlgorithmIndex];
    processes = originalProcesses;
    std::vector<ExecutionSlice> timeline;

    if (mainGanttChart) {
        mainGanttChart->setAlgorithmTitle(algorithmName);
    }

    if (algorithmName == "FIFO") {
        timeline = SchedulingAlgorithms::runFIFO(processes);
    } else if (algorithmName == "SJF") {
        timeline = SchedulingAlgorithms::runSJF(processes);
    } else if (algorithmName == "SRTF") {
        timeline = SchedulingAlgorithms::runSRT(processes);
    } else if (algorithmName == "Round Robin") {
        timeline = SchedulingAlgorithms::runRoundRobin(processes, selectedQuantum);
    } else if (algorithmName == "Priority") {
        timeline = SchedulingAlgorithms::runPriority(processes, agingEnabled, selectedAging);
    }

    // GUARDAR RESULTADO EN sequentialResults
    double totalWait = 0, totalTurnaround = 0;
    for (const auto& p : processes) {
        totalWait += p.waiting_time;
        totalTurnaround += (p.finish_time - p.arrival_time);
    }
    double totalCompletion = 0;
    for (const auto& process : processes) {
        totalCompletion += process.finish_time;
    }
    double avgWaiting = processes.empty() ? 0 : totalWait / processes.size();
    double avgTurnaround = processes.empty() ? 0 : totalTurnaround / processes.size();
    double avgCompletion = processes.empty() ? 0 : totalCompletion / processes.size(); // Nuevo cálculo

    if (metricsLabelBelowGantt) {
        metricsLabelBelowGantt->setText(
            QString("Avg Waiting Time: %1 | Avg Turnaround Time: %2 | Avg Completion Time: %3")
                .arg(avgWaiting, 0, 'f', 2)
                .arg(avgTurnaround, 0, 'f', 2)
                .arg(avgCompletion, 0, 'f', 2)
        );
    }

    SimulationResult result;
    result.algorithmName = algorithmName;
    result.avgWaitingTime = avgWaiting;
    result.avgTurnaroundTime = avgTurnaround;
    result.avgCompletionTime = avgCompletion; 
    result.timeline = timeline;
    result.processResults = processes;
    sequentialResults.push_back(result);

    if (mainGanttChart) {
        mainGanttChart->setComparisonMode(false);
        mainGanttChart->setTimeline(timeline);
        mainGanttChart->startAnimation();
    }

    currentAlgorithmIndex++;
    updateMetricsTable();
}

void ProcessSimulator::displayAlgorithmResultInList(const QString& title, 
                                                const std::vector<ExecutionSlice>& timeline, 
                                                const std::vector<Process>& processResults) {

    QLayoutItem* item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    QFrame* resultFrame = new QFrame();
    resultFrame->setFrameStyle(QFrame::Box);
    resultFrame->setStyleSheet("QFrame { border: 2px solid #e9ecef; border-radius: 8px; margin: 5px; padding: 10px; background-color: white; }");
    
    QVBoxLayout* frameLayout = new QVBoxLayout(resultFrame);

    QLabel* header = new QLabel(title);
    header->setFont(QFont("Arial", 16, QFont::Bold));
    header->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    header->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(header);

    GanttChartWidget* chart = new GanttChartWidget();
    chart->setTimeline(timeline);
    chart->setFixedHeight(220);
    frameLayout->addWidget(chart);

    double totalWaiting = 0;
    double totalTurnaround = 0;
    for (const auto& process : processResults) {
        totalWaiting += process.waiting_time;
        totalTurnaround += (process.finish_time - process.arrival_time);
    }

    double totalCompletion = 0;
    for (const auto& process : processResults) {
        totalCompletion += process.finish_time;
    }
    double avgWaiting = processResults.empty() ? 0 : totalWaiting / processResults.size();
    double avgTurnaround = processResults.empty() ? 0 : totalTurnaround / processResults.size();
    double avgCompletion = processResults.empty() ? 0 : totalCompletion / processResults.size();

    QLabel* summary = new QLabel(QString("Avg Waiting Time: %1 | Avg Turnaround Time: %2 | Avg Completion Time: %3")
    .arg(avgWaiting, 0, 'f', 2)
    .arg(avgTurnaround, 0, 'f', 2)
    .arg(avgCompletion, 0, 'f', 2));
    summary->setFont(QFont("Arial", 12, QFont::Bold));
    summary->setStyleSheet("color: #28a745; margin-top: 5px;");
    summary->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(summary);
    resultsLayout->addWidget(resultFrame);
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
        metricsTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].finish_time))); // Completion Time
        metricsTable->setItem(i, 4, new QTableWidgetItem(QString::number(processes[i].waiting_time)));
        totalWaiting += processes[i].waiting_time;
        totalTurnaround += (processes[i].finish_time - processes[i].arrival_time);
    }
    double avgWaiting = totalWaiting / processes.size();
    double avgTurnaround = totalTurnaround / processes.size();

    if (mainGanttChart) {
        mainGanttChart->setMetrics(avgWaiting, avgTurnaround);
    }
}

void ProcessSimulator::loadProcessesFromDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Processes", "", "Text Files (*.txt)");
    if (fileName.isEmpty())
        return;

    QFileInfo fileInfo(fileName);
    if (fileInfo.fileName() != "processes.txt") {
        QMessageBox::warning(this, "Nombre de archivo inválido", "Solo se permite cargar archivos llamados exactamente 'processes.txt'.");
        return;
    }

    processes = loadProcesses(fileName);
    originalProcesses = processes; 
    updateProcessTable();
    statusLabel->setText(QString("Loaded %1 processes from file").arg(processes.size()));

    if (mainGanttChart) {
        mainGanttChart->setTimeline(std::vector<ExecutionSlice>());
    }

    QMessageBox::information(this, "Archivo cargado", QString("Se cargaron %1 procesos desde %2").arg(processes.size()).arg(fileName));
}

void ProcessSimulator::setupUI(QWidget* menuWidget_)
{
    setWindowTitle("Advanced Process Scheduling & Synchronization Simulator");
    setFixedSize(1400, 1000);
    setStyleSheet("background-color: #f8f9fa;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainStack = new QStackedWidget();
    setupSchedulingWidget(menuWidget_);
    setupMultiSelectionWidget();
    setupSequentialSimWidget(menuWidget_);
    mainLayout->addWidget(mainStack);
}


void ProcessSimulator::setupMultiSelectionWidget() {
    multiSelectionWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(multiSelectionWidget);

    QLabel *title = new QLabel("Selecciona los algoritmos que deseas simular:");
    title->setFont(QFont("Arial", 16, QFont::Bold));
    layout->addWidget(title);

    // Checkboxes de algoritmos
    fifoCheckMulti = createStyledCheckBox("FIFO", "#FFE4E1");        
    sjfCheckMulti = createStyledCheckBox("SJF", "#E0F6FF");          
    srtfCheckMulti = createStyledCheckBox("SRTF", "#F0FFF0");       
    rrCheckMulti = createStyledCheckBox("Round Robin", "#FFF8DC");    
    priorityCheckMulti = createStyledCheckBox("Priority", "#F0E68C"); 

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

    QPushButton *loadBtn = createButton("Cargar Procesos", "#5a68a5");
    connect(loadBtn, &QPushButton::clicked, this, [this]() {
        loadProcessesFromDialog();
        originalProcesses = processes;
        QMessageBox::information(this, "Carga Exitosa", "Procesos cargados correctamente.");
    });
    layout->addWidget(loadBtn);
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
        sequentialResults.clear(); // Limpiar resultados de simulaciones anteriores
        mainStack->setCurrentWidget(sequentialSimWidget);
        simulateNextAlgorithm();
    });

    mainStack->addWidget(multiSelectionWidget);
}

void ProcessSimulator::setupSchedulingWidget(QWidget* menuWidget_)
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
    QPushButton *cleanBtn = createButton("Clear", "#dc3545"); 
    processLayout->addWidget(loadBtn);
    processLayout->addWidget(generateBtn);
    processLayout->addWidget(cleanBtn); 
    processLayout->addStretch();

    mainGanttChart = new GanttChartWidget();  
    QScrollArea *mainGanttScrollArea = mainGanttChart->createScrollArea();

    connect(mainGanttChart, &GanttChartWidget::animationFinished,
        this, &ProcessSimulator::runNextAlgorithmInSequence);
    
    //QHBoxLayout *animLayout = new QHBoxLayout();
    //QPushButton *startBtn = createButton("▶ Start Animation", "#30c752");
    //QPushButton *stopBtn = createButton("Stop Animation", "#dc3545");
    //QSpinBox *speedSpinBox = new QSpinBox();
    //speedSpinBox->setRange(100, 2000);
    //speedSpinBox->setValue(500);
    //speedSpinBox->setSuffix(" ms");
    //animLayout->addWidget(new QLabel("Speed:"));
    //animLayout->addWidget(speedSpinBox);
    //animLayout->addWidget(startBtn);
    //animLayout->addWidget(stopBtn);
    //animLayout->addStretch();

    QHBoxLayout *tablesLayout = new QHBoxLayout();
    processTable = new QTableWidget();
    processTable->setColumnCount(5);
    processTable->setHorizontalHeaderLabels({"PID", "Burst Time", "Arrival Time", "Priority", "Color"});
    setupTableStyle(processTable);

    metricsTable = new QTableWidget();
    metricsTable->setColumnCount(5);
    metricsTable->setHorizontalHeaderLabels({"PID", "Start Time", "Finish Time", "Completion Time", "Waiting Time"});
    setupTableStyle(metricsTable);
    metricsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    tablesLayout->addWidget(processTable);
    tablesLayout->addWidget(metricsTable);

    statusLabel = new QLabel("Ready to simulate scheduling algorithms");
    statusLabel->setStyleSheet("color: #6c757d; padding: 10px; font-size: 14px;");

    layout->addLayout(headerLayout);
    layout->addLayout(processLayout);
    setupAlgorithmSelection(layout);
    layout->addWidget(mainGanttScrollArea); 
    //layout->addLayout(animLayout);
    layout->addLayout(tablesLayout);
    layout->addWidget(statusLabel);

    metricsLabelBelowGantt = new QLabel();
    metricsLabelBelowGantt->setAlignment(Qt::AlignCenter);
    metricsLabelBelowGantt->setStyleSheet("color: #28a745; font-weight: bold; font-size: 16px;");
    layout->addWidget(metricsLabelBelowGantt);


    connect(backBtn, &QPushButton::clicked, this, &ProcessSimulator::returnToMenuRequested);
    connect(loadBtn, &QPushButton::clicked, this, &ProcessSimulator::loadProcessesFromDialog);
    connect(generateBtn, &QPushButton::clicked, this, &ProcessSimulator::generateSampleProcesses);
    connect(cleanBtn, &QPushButton::clicked, this, &ProcessSimulator::cleanProcesses); 
    //connect(startBtn, &QPushButton::clicked, mainGanttChart, &GanttChartWidget::startAnimation);
    //connect(stopBtn, &QPushButton::clicked, mainGanttChart, &GanttChartWidget::stopAnimation);
    //connect(speedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), mainGanttChart, &GanttChartWidget::setAnimationSpeed);

    mainStack->addWidget(schedulingWidget);
}


void ProcessSimulator::setupSequentialSimWidget(QWidget* menuWidget_) {
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

    connect(returnBtn, &QPushButton::clicked, [this, menuWidget_]() {
        mainStack->setCurrentWidget(menuWidget_);
    });

    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &ProcessSimulator::simulateNextAlgorithm);

    mainStack->addWidget(sequentialSimWidget);
}

// 2. Modificación de simulateNextAlgorithm para guardar resultados y mostrar resumen
void ProcessSimulator::simulateNextAlgorithm() {
    if (selectedAlgorithms.empty()) {
        showSimulationSummary(menuWidget_); 
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

    // Calcular métricas
    double totalWait = 0, totalTurnaround = 0;
    for (const auto& p : processes) {
        totalWait += p.waiting_time;
        totalTurnaround += (p.finish_time - p.arrival_time);
    }
    double totalCompletion = 0;
    for (const auto& process : processes) {
        totalCompletion += process.finish_time;
    }

    double avgWaiting = totalWait / processes.size();
    double avgTurnaround = totalTurnaround / processes.size();
    double avgCompletion = avgTurnaround; // El mismo valor en este contexto

    metricsLabel->setText(QString("Avg Waiting Time: %1 | Avg Turnaround Time: %2 | Avg Completion Time: %3")
        .arg(avgWaiting, 0, 'f', 2)
        .arg(avgTurnaround, 0, 'f', 2)
        .arg(avgCompletion, 0, 'f', 2));

    // Guardar resultado para el resumen
    SimulationResult result;
    result.algorithmName = config.name;
    result.avgWaitingTime = avgWaiting;
    result.avgTurnaroundTime = avgTurnaround;
    result.avgCompletionTime = avgCompletion;
    result.timeline = timeline;
    result.processResults = processes;
    sequentialResults.push_back(result);

    int fixedDuration = 4000; // 4 segundos mínimo por algoritmo
    simulationTimer->start(fixedDuration);
}

// 3. Nueva función para mostrar ventana de resumen
void ProcessSimulator::showSimulationSummary(QWidget* menuWidget_) {
    if (sequentialResults.empty()) {
        QMessageBox::warning(this, "Sin resultados", "No hay simulaciones finalizadas para mostrar.");
        return;
    }
    resultsWindow = new QWidget();
    resultsWindow->setWindowTitle("Resumen de Simulaciones");
    resultsWindow->setFixedSize(1200, 800);
    resultsWindow->setStyleSheet("background-color: #f8f9fa;");
    
    resultsWindowLayout = new QVBoxLayout(resultsWindow);
    
    // Título
    QLabel* title = new QLabel("RESUMEN DE SIMULACIONES");
    title->setFont(QFont("Arial", 20, QFont::Bold));
    title->setStyleSheet("color: #2c3e50; margin: 20px 0;");
    title->setAlignment(Qt::AlignCenter);
    resultsWindowLayout->addWidget(title);
    
    // Tabla de comparación
    createComparisonTable();
    
    // Área con scroll para los Gantt charts individuales
    QScrollArea* ganttScrollArea = new QScrollArea();
    QWidget* ganttContainer = new QWidget();
    QVBoxLayout* ganttLayout = new QVBoxLayout(ganttContainer);
    
    // Agregar cada resultado individual
    for (const auto& result : sequentialResults) {
        createIndividualResultWidget(result, ganttLayout);
    }
    
    ganttContainer->setLayout(ganttLayout);
    ganttScrollArea->setWidget(ganttContainer);
    ganttScrollArea->setWidgetResizable(true);
    ganttScrollArea->setMaximumHeight(400);
    
    resultsWindowLayout->addWidget(ganttScrollArea);
    
    // Botones
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* closeBtn = createButton("Cerrar", "#dc3545");
    QPushButton* backToMenuBtn = createButton("Volver al Menú Principal", "#6c757d");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    buttonLayout->addWidget(backToMenuBtn);
    buttonLayout->addStretch();
    
    resultsWindowLayout->addLayout(buttonLayout);
    
    // Conectar botones
    connect(closeBtn, &QPushButton::clicked, resultsWindow, &QWidget::close);
    connect(backToMenuBtn, &QPushButton::clicked, [this, menuWidget_]() {
        resultsWindow->close();
        mainStack->setCurrentWidget(menuWidget_);
    });
    
    resultsWindow->show();
}

void ProcessSimulator::createComparisonTable() {
    QLayoutItem* item;
    while ((item = resultsWindowLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QLabel* tableTitle = new QLabel("Tabla de Comparación de Algoritmos");
    tableTitle->setFont(QFont("Arial", 20, QFont::Bold));
    tableTitle->setStyleSheet("color: #2c3e50; margin: 10px 0;");
    tableTitle->setAlignment(Qt::AlignCenter);
    resultsWindowLayout->addWidget(tableTitle);
    
    QTableWidget* comparisonTable = new QTableWidget();
    comparisonTable->setRowCount(sequentialResults.size());
    comparisonTable->setColumnCount(4); // Cambiado a 4 columnas
    comparisonTable->setHorizontalHeaderLabels({"Algoritmo", "Tiempo Promedio de Espera", "Tiempo Promedio de Retorno", "Tiempo Promedio de Finalización"});
    setupTableStyle(comparisonTable);
    
    double bestWaiting = std::numeric_limits<double>::max();
    double bestTurnaround = std::numeric_limits<double>::max();
    
    for (const auto& result : sequentialResults) {
        if (result.avgWaitingTime < bestWaiting) bestWaiting = result.avgWaitingTime;
        if (result.avgTurnaroundTime < bestTurnaround) bestTurnaround = result.avgTurnaroundTime;
    }
    
    for (int i = 0; i < sequentialResults.size(); i++) {
        const auto& result = sequentialResults[i];

        comparisonTable->setItem(i, 0, new QTableWidgetItem(result.algorithmName));

        QTableWidgetItem* waitingItem = new QTableWidgetItem(QString::number(result.avgWaitingTime, 'f', 2));
        QTableWidgetItem* turnaroundItem = new QTableWidgetItem(QString::number(result.avgTurnaroundTime, 'f', 2));
        QTableWidgetItem* completionItem = new QTableWidgetItem(QString::number(result.avgCompletionTime, 'f', 2)); // Nuevo item

        if (result.avgWaitingTime == bestWaiting) {
            waitingItem->setBackground(QColor("#d4edda")); 
            waitingItem->setForeground(QColor("#155724")); 
        }
        if (result.avgTurnaroundTime == bestTurnaround) {
            turnaroundItem->setBackground(QColor("#d4edda"));
            turnaroundItem->setForeground(QColor("#155724"));
        }

        comparisonTable->setItem(i, 1, waitingItem);
        comparisonTable->setItem(i, 2, turnaroundItem);
        comparisonTable->setItem(i, 3, completionItem); // Agregar item de finalización

        QColor rowColor = Qt::white;
        if (result.algorithmName == "FIFO") rowColor = QColor("#FFE4E1");
        else if (result.algorithmName == "SJF") rowColor = QColor("#E0F6FF");
        else if (result.algorithmName == "SRTF") rowColor = QColor("#F0FFF0");
        else if (result.algorithmName == "Round Robin" || result.algorithmName == "RR") rowColor = QColor("#FFF8DC");
        else if (result.algorithmName == "Priority" || result.algorithmName == "PRIORITY") rowColor = QColor("#F0E68C");

        for (int j = 0; j < 4; ++j) {
            if (comparisonTable->item(i, j))
                comparisonTable->item(i, j)->setBackground(rowColor);
        }
    }
    
    comparisonTable->resizeColumnsToContents();
    comparisonTable->setMinimumWidth(900);
    comparisonTable->setMinimumHeight(300);
    comparisonTable->setMaximumHeight(400);
    comparisonTable->verticalHeader()->setDefaultSectionSize(40);
    comparisonTable->setFont(QFont("Arial", 16));
    comparisonTable->setStyleSheet("background-color: white; border: 1px solid #ddd; font-size: 18px;"); 
    comparisonTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    resultsWindowLayout->addWidget(comparisonTable);
}

// 5. Función para crear widgets individuales de resultados
void ProcessSimulator::createIndividualResultWidget(const SimulationResult& result, QVBoxLayout* layout) {
    QFrame* resultFrame = new QFrame();
    resultFrame->setFrameStyle(QFrame::Box);
    resultFrame->setStyleSheet("QFrame { border: 2px solid #e9ecef; border-radius: 8px; margin: 5px; padding: 10px; background-color: white; }");

    QVBoxLayout* frameLayout = new QVBoxLayout(resultFrame);

    // Header con título del algoritmo
    QLabel* header = new QLabel(result.algorithmName);
    header->setFont(QFont("Arial", 16, QFont::Bold));
    header->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    header->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(header);

    // Crear un nuevo chart para mostrar en los resultados
    GanttChartWidget* chart = new GanttChartWidget();
    chart->setTimeline(result.timeline);
    chart->updateSize(); // <-- Asegura el tamaño correcto

    // Ajusta el ancho del chart según el tiempo máximo
    int maxTime = 0;
    for (const auto& slice : result.timeline) {
        maxTime = std::max(maxTime, slice.start_time + slice.duration);
    }
    int minWidth = maxTime * 40 + 100;
    chart->setMinimumWidth(minWidth);
    chart->setFixedHeight(220);

    // Poner el chart en un QScrollArea para scroll horizontal si es necesario
    QScrollArea* chartScroll = new QScrollArea();
    chartScroll->setWidget(chart);
    chartScroll->setWidgetResizable(false); // <-- Importante para scroll horizontal
    chartScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chartScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chartScroll->setFixedHeight(240);

    frameLayout->addWidget(chartScroll);

    QLabel* summary = new QLabel(QString("Avg Waiting Time: %1 | Avg Turnaround Time: %2 | Avg Completion Time: %3")
        .arg(result.avgWaitingTime, 0, 'f', 2)
        .arg(result.avgTurnaroundTime, 0, 'f', 2)
        .arg(result.avgCompletionTime, 0, 'f', 2));
    summary->setFont(QFont("Arial", 12, QFont::Bold));
    summary->setStyleSheet("color: #28a745; margin-top: 5px;");
    summary->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(summary);

    layout->addWidget(resultFrame);
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
    // Limpiar resultados previos
    QLayoutItem* item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    algorithmTimelines.clear();
    algorithmNames.clear();
    std::vector<Process> originalProcessesCopy = originalProcesses;

    if (originalProcessesCopy.empty()) {
        QMessageBox::warning(this, "No hay procesos", "Por favor carga procesos antes de comparar algoritmos.");
        return;
    }

    QStringList selectedAlgs;
    std::vector<double> waitingTimes;
    std::vector<double> turnaroundTimes;
    
    if (fifoCheck->isChecked()) {
        processes = originalProcessesCopy;
        SchedulingAlgorithms::runFIFO(processes);
        selectedAlgs.append("FIFO");
        
        double totalWaiting = 0, totalTurnaround = 0;
        for (const auto& process : processes) {
            totalWaiting += process.waiting_time;
            totalTurnaround += (process.finish_time - process.arrival_time);
        }
        waitingTimes.push_back(totalWaiting / processes.size());
        turnaroundTimes.push_back(totalTurnaround / processes.size());
    }
    
    if (sjfCheck->isChecked()) {
        processes = originalProcessesCopy;
        SchedulingAlgorithms::runSJF(processes);
        selectedAlgs.append("SJF");
        
        double totalWaiting = 0, totalTurnaround = 0;
        for (const auto& process : processes) {
            totalWaiting += process.waiting_time;
            totalTurnaround += (process.finish_time - process.arrival_time);
        }
        waitingTimes.push_back(totalWaiting / processes.size());
        turnaroundTimes.push_back(totalTurnaround / processes.size());
    }
    
    if (srtfCheck->isChecked()) {
        processes = originalProcessesCopy;
        SchedulingAlgorithms::runSRT(processes);
        selectedAlgs.append("SRTF");
        
        double totalWaiting = 0, totalTurnaround = 0;
        for (const auto& process : processes) {
            totalWaiting += process.waiting_time;
            totalTurnaround += (process.finish_time - process.arrival_time);
        }
        waitingTimes.push_back(totalWaiting / processes.size());
        turnaroundTimes.push_back(totalTurnaround / processes.size());
    }
    
    if (rrCheck->isChecked()) {
        processes = originalProcessesCopy;
        SchedulingAlgorithms::runRoundRobin(processes, selectedQuantum);
        selectedAlgs.append("Round Robin");
        
        double totalWaiting = 0, totalTurnaround = 0;
        for (const auto& process : processes) {
            totalWaiting += process.waiting_time;
            totalTurnaround += (process.finish_time - process.arrival_time);
        }
        waitingTimes.push_back(totalWaiting / processes.size());
        turnaroundTimes.push_back(totalTurnaround / processes.size());
    }
    
    if (priorityCheck->isChecked()) {
        processes = originalProcessesCopy;
        SchedulingAlgorithms::runPriority(processes, agingEnabled, selectedAging);
        selectedAlgs.append("Priority");
        
        double totalWaiting = 0, totalTurnaround = 0;
        for (const auto& process : processes) {
            totalWaiting += process.waiting_time;
            totalTurnaround += (process.finish_time - process.arrival_time);
        }
        waitingTimes.push_back(totalWaiting / processes.size());
        turnaroundTimes.push_back(totalTurnaround / processes.size());
    }
    
    if (!selectedAlgs.isEmpty()) {
        displayComparisonTableOnly(selectedAlgs, waitingTimes, turnaroundTimes);
    } else {
        QMessageBox::warning(this, "No hay algoritmos", "Por favor selecciona al menos un algoritmo para comparar.");
    }
    
    // Restaurar procesos originales
    processes = originalProcessesCopy;
    updateProcessTable();
}

// 5. Nueva función para mostrar solo tabla de comparación
void ProcessSimulator::displayComparisonTableOnly(const QStringList& algorithms, 
                                                const std::vector<double>& waitingTimes,
                                                const std::vector<double>& turnaroundTimes) {
    // LIMPIAR resultados previos
    QLayoutItem* item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QLabel* header = new QLabel("Comparación de Algoritmos");
    header->setFont(QFont("Arial", 22, QFont::Bold));
    header->setStyleSheet("color: #2c3e50; margin: 20px 0;");
    header->setAlignment(Qt::AlignCenter);
    resultsLayout->addWidget(header);

    QTableWidget* comparisonTable = new QTableWidget();
    comparisonTable->setRowCount(algorithms.size());
    comparisonTable->setColumnCount(4); // Cambiado a 4 columnas
    comparisonTable->setHorizontalHeaderLabels({"Algoritmo", "Tiempo Promedio de Espera", "Tiempo Promedio de Retorno", "Tiempo Promedio de Finalización"});
    setupTableStyle(comparisonTable);

    for (int i = 0; i < algorithms.size(); i++) {
        comparisonTable->setItem(i, 0, new QTableWidgetItem(algorithms[i]));
        comparisonTable->setItem(i, 1, new QTableWidgetItem(QString::number(waitingTimes[i], 'f', 2)));
        comparisonTable->setItem(i, 2, new QTableWidgetItem(QString::number(turnaroundTimes[i], 'f', 2)));
        
        // Colores según algoritmo
        QColor rowColor;
        if (algorithms[i] == "FIFO") rowColor = QColor("#FFE4E1");
        else if (algorithms[i] == "SJF") rowColor = QColor("#E0F6FF");
        else if (algorithms[i] == "SRTF") rowColor = QColor("#F0FFF0");
        else if (algorithms[i] == "Round Robin") rowColor = QColor("#FFF8DC");
        else if (algorithms[i] == "Priority") rowColor = QColor("#F0E68C");
        else rowColor = QColor(Qt::white);

        for (int j = 0; j < 4; j++) {
            comparisonTable->item(i, j)->setBackground(rowColor);
        }
    }
    
    comparisonTable->resizeColumnsToContents();
    comparisonTable->setMinimumWidth(900);
    comparisonTable->setMinimumHeight(300);
    comparisonTable->setMaximumHeight(400);
    comparisonTable->setFont(QFont("Arial", 16)); 
    comparisonTable->setStyleSheet("background-color: white; border: 1px solid #ddd; font-size: 22px;");
    comparisonTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    resultsLayout->addWidget(comparisonTable);
    
    statusLabel->setText(QString("Comparación completada para %1 algoritmos").arg(algorithms.size()));
}

void ProcessSimulator::cleanProcesses()
{
    processes.clear();
    originalProcesses.clear();
    processTable->setRowCount(0);
    metricsTable->setRowCount(0);
    if (mainGanttChart) {
        mainGanttChart->setTimeline(std::vector<ExecutionSlice>());
    }
    QLayoutItem* item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    statusLabel->setText("Processes cleared. Ready to load new processes.");
}

void ProcessSimulator::setupTableStyle(QTableWidget* table) {
    table->setStyleSheet(
        "QTableWidget {"
        "  background-color: #f8f9fa;"
        "  border: 2px solidrgb(196, 176, 197);"
        "  border-radius: 10px;"
        "  font-size: 16px;"
        "  selection-background-color:rgb(228, 178, 242);"
        "  selection-color: #263238;"
        "}"
        "QHeaderView::section {"
        "  background-color:rgb(113, 25, 96);"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 16px;"
        "  border: none;"
        "  padding: 8px;"
        "}"
        "QTableWidget::item {"
        "  padding: 6px;"
        "}"
    );
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
}