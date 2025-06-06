#include "synchronizer.h"
#include "synchronizationsimulator.h"
#include "loader.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QTimer>
#include <QScrollArea>
#include <QFont>
#include <QAbstractItemView>
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>
#include <QScrollBar>
#include <climits>
#include <QGraphicsDropShadowEffect>

SynchronizationSimulatorWidget::SynchronizationSimulatorWidget(QStackedWidget* mainStack, QWidget* menuWidget, QWidget *parent)
    : QWidget(parent), mainStack_(mainStack), menuWidget_(menuWidget), syncMechanism(nullptr), 
      currentSyncType("Mutex"), currentAnimationCycle(0), maxCycles(0)
{
    setupUI();
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &SynchronizationSimulatorWidget::nextAnimationStep);
}

SynchronizationSimulatorWidget::~SynchronizationSimulatorWidget()
{
    delete syncMechanism;
}

void SynchronizationSimulatorWidget::setupUI()
{
    setWindowTitle("Synchronization Simulator");
    setFixedSize(1400, 900);
    setStyleSheet("background-color: #f8f9fa;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backBtn = createButton("← Back to Menu", "#6c757d");
    QLabel *title = new QLabel("SIMULADOR DE MECANISMOS DE SINCRONIZACIÓN");
    title->setFont(QFont("Arial", 16, QFont::Bold));
    title->setStyleSheet(
        "color: #2c3e50;"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #c4e5fb, stop:1 #e3f0fc);"
        "border-radius: 8px;"
        "padding: 8px;"
        "margin: 2px;"
    );
    title->setAlignment(Qt::AlignCenter);
    title->setMaximumHeight(40);
    
    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(title, 1);
    headerLayout->addStretch();

    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    QLabel *typeLabel = new QLabel("Tipo:");
    typeLabel->setFont(QFont("Arial", 10, QFont::Bold));
    
    syncTypeCombo = new QComboBox();
    syncTypeCombo->addItems({"Mutex", "Semaphore"});
    syncTypeCombo->setStyleSheet(
        "QComboBox { "
        "  padding: 4px; "
        "  font-size: 10px; "
        "  border-radius: 4px; "
        "  border: 1px solid #ced4da; "
        "  background: white; "
        "}"
    );
    syncTypeCombo->setMaximumHeight(30);

    loadResBtn = createButton("Cargar Recursos", "#70a1a8");
    loadActBtn = createButton("Cargar Acciones", "#28a745");
    QPushButton *runMutexBtn = createButton("Simular Mutex", "#dc3545");
    QPushButton *runSemBtn = createButton("Simular Semáforo", "#ffc107");
    QPushButton *clearBtn = createButton("Limpiar", "#6c757d");
    QPushButton *infoBtn = createButton("Mostrar Info", "#17a2b8");

    controlLayout->addWidget(typeLabel);
    controlLayout->addWidget(syncTypeCombo);
    controlLayout->addWidget(loadResBtn);
    controlLayout->addWidget(loadActBtn);
    controlLayout->addWidget(runMutexBtn);
    controlLayout->addWidget(runSemBtn);
    controlLayout->addWidget(clearBtn);
    controlLayout->addWidget(infoBtn);
    controlLayout->addStretch();

    QHBoxLayout *statusLayout = new QHBoxLayout();
    
    statusLabel = new QLabel("Mutex seleccionado - Solo necesitas cargar acciones para iniciar la simulación.");
    statusLabel->setStyleSheet(
        "color: #495057; "
        "background: #e9ecef; "
        "border: 1px solid #ced4da; "
        "border-radius: 4px; "
        "padding: 6px; "
        "font-size: 10px;"
    );
    statusLabel->setMaximumHeight(25);
    statusLabel->setWordWrap(true);
    
    cycleLabel = new QLabel("Ciclo Actual: 0");
    cycleLabel->setFont(QFont("Arial", 12, QFont::Bold));
    cycleLabel->setStyleSheet(
        "color: #2c3e50; "
        "background: white; "
        "border: 2px solid #3498db; "
        "border-radius: 6px; "
        "padding: 6px; "
    );
    cycleLabel->setAlignment(Qt::AlignCenter);
    cycleLabel->setMinimumWidth(120);
    cycleLabel->setMaximumHeight(30);
    
    statusLayout->addWidget(statusLabel, 3);
    statusLayout->addWidget(cycleLabel, 1);

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setHandleWidth(8);
    
    QWidget *leftPanel = new QWidget();
    leftPanel->setMaximumWidth(320);
    leftPanel->setMinimumWidth(300);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(6);
    
    QGroupBox *infoGroup = new QGroupBox("Información del Sistema");
    infoGroup->setFont(QFont("Arial", 10, QFont::Bold));
    infoGroup->setMaximumHeight(100);
    infoGroup->setStyleSheet(
        "QGroupBox { "
        "  font-weight: bold; "
        "  border: 1px solid #bdc3c7; "
        "  border-radius: 6px; "
        "  margin-top: 8px; "
        "  background: #ecf0f1; "
        "} "
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 8px; "
        "  padding: 0 6px 0 6px; "
        "  background: white; "
        "  border-radius: 3px; "
        "}"
    );
    QVBoxLayout *infoGroupLayout = new QVBoxLayout(infoGroup);
    infoGroupLayout->setContentsMargins(6, 12, 6, 6);
    infoDisplay = new QTextEdit();
    infoDisplay->setMaximumHeight(70);
    infoDisplay->setStyleSheet(
        "background-color: #e8f4fd; "
        "border: 1px solid #2196f3; "
        "border-radius: 4px; "
        "padding: 4px; "
        "font-family: 'Consolas', monospace; "
        "font-size: 9px;"
    );
    infoDisplay->setReadOnly(true);
    infoGroupLayout->addWidget(infoDisplay);
    leftLayout->addWidget(infoGroup);
    
    QGroupBox *eventsGroup = new QGroupBox("Eventos de Simulación");
    eventsGroup->setFont(QFont("Arial", 10, QFont::Bold));
    eventsGroup->setStyleSheet(
        "QGroupBox { "
        "  font-weight: bold; "
        "  border: 1px solid #bdc3c7; "
        "  border-radius: 6px; "
        "  margin-top: 8px; "
        "  background: #ecf0f1; "
        "} "
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 8px; "
        "  padding: 0 6px 0 6px; "
        "  background: white; "
        "  border-radius: 3px; "
        "}"
    );
    QVBoxLayout *eventsLayout = new QVBoxLayout(eventsGroup);
    eventsLayout->setContentsMargins(6, 12, 6, 6);
    
    syncTable = new QTableWidget();
    syncTable->setColumnCount(5);
    syncTable->setHorizontalHeaderLabels({"Proceso", "Estado", "Recurso", "Acción", "Ciclo"});
    
    syncTable->setMinimumHeight(520);
    syncTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #ffffff;"
        "  border: 1px solid #3498db;"
        "  border-radius: 6px;"
        "  font-size: 10px;"
        "  selection-background-color: #e3f0fc;"
        "  gridline-color: #ecf0f1;"
        "  margin: 2px;"
        "}"
        "QHeaderView::section {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 10px;"
        "  border: none;"
        "  padding: 8px;"
        "  border-right: 1px solid #2980b9;"
        "}"
        "QTableWidget::item {"
        "  padding: 6px;"
        "  border-bottom: 1px solid #ecf0f1;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #d4edda;"
        "  color: #155724;"
        "}"
    );
    
    syncTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    syncTable->horizontalHeader()->setStretchLastSection(true);
    syncTable->verticalHeader()->setVisible(false);
    syncTable->setShowGrid(true);
    syncTable->setAlternatingRowColors(true);
    syncTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    syncTable->setColumnWidth(0, 60);
    syncTable->setColumnWidth(1, 70);
    syncTable->setColumnWidth(2, 60);
    syncTable->setColumnWidth(3, 60);
    
    eventsLayout->addWidget(syncTable);
    leftLayout->addWidget(eventsGroup);
    
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(4);
    
    QGroupBox *timelineGroup = new QGroupBox("Timeline Horizontal de Sincronización");
    timelineGroup->setFont(QFont("Arial", 11, QFont::Bold));
    timelineGroup->setStyleSheet(
        "QGroupBox { "
        "  font-weight: bold; "
        "  border: 1px solid #2c3e50; "
        "  border-radius: 8px; "
        "  margin-top: 10px; "
        "  background: #ecf0f1; "
        "} "
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 12px; "
        "  padding: 0 8px 0 8px; "
        "  background: white; "
        "  border-radius: 4px; "
        "  color: #2c3e50; "
        "}"
    );
    QVBoxLayout *timelineLayout = new QVBoxLayout(timelineGroup);
    timelineLayout->setContentsMargins(8, 15, 8, 8);
    
    simulationArea = new QWidget();
    simulationArea->setMinimumHeight(600);
    simulationArea->setStyleSheet(
        "background: #ffffff; "
        "border: 1px solid #3498db; "
        "border-radius: 6px;"
    );
    
    QScrollArea *timelineScroll = new QScrollArea();
    timelineScroll->setWidget(simulationArea);
    timelineScroll->setWidgetResizable(false);
    timelineScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    timelineScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineScroll->setMinimumHeight(720);
    timelineScroll->setMaximumHeight(720);

    timelineGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    timelineLayout->addWidget(timelineScroll);
    rightLayout->addWidget(timelineGroup);
    
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({300, 1100});

    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(statusLayout);
    mainLayout->addWidget(mainSplitter);

    connect(backBtn, &QPushButton::clicked, [this]() {
        emit backToMenuRequested();
    });
    connect(syncTypeCombo, &QComboBox::currentTextChanged, this, &SynchronizationSimulatorWidget::onSyncTypeChanged);
    connect(loadResBtn, &QPushButton::clicked, [this]() {
        if (currentSyncType == "Mutex") {
            QMessageBox::warning(this, "No requerido", "No necesitas cargar recursos para Mutex.");
        } else {
            loadResourcesFromDialog();
        }
    });
    connect(loadActBtn, &QPushButton::clicked, this, &SynchronizationSimulatorWidget::loadActionsFromDialog);
    connect(runMutexBtn, &QPushButton::clicked, [this]() { runSynchronization("Mutex Lock"); });
    connect(runSemBtn, &QPushButton::clicked, [this]() { runSynchronization("Semaphore"); });
    connect(clearBtn, &QPushButton::clicked, this, &SynchronizationSimulatorWidget::clearAll);
    connect(infoBtn, &QPushButton::clicked, this, &SynchronizationSimulatorWidget::showInfo);
    
    onSyncTypeChanged();
}

QPushButton* SynchronizationSimulatorWidget::createButton(const QString &text, const QString &color)
{
    QPushButton *btn = new QPushButton(text);
    btn->setMinimumSize(180, 45);
    btn->setFont(QFont("Arial", 14, QFont::Bold));
    btn->setStyleSheet(QString(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:1 %2); "
        "  color: white; "
        "  border-radius: 8px; "
        "  padding: 8px; "
        "  font-size: 11px; "
        "  border: 2px solid %3; "
        "  margin: 3px; "
        "}"
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %4, stop:1 %5); "
        "  margin-top: 2px; "
        "  margin-bottom: 4px; "
        "}"
        "QPushButton:pressed { "
        "  background: %6; "
        "  margin-top: 4px; "
        "  margin-bottom: 2px; "
        "}"
    ).arg(color)
     .arg(QColor(color).darker(110).name())
     .arg(QColor(color).darker(130).name())
     .arg(QColor(color).lighter(110).name())
     .arg(QColor(color).name())
     .arg(QColor(color).darker(120).name()));
    return btn;
}

void SynchronizationSimulatorWidget::onSyncTypeChanged()
{
    currentSyncType = syncTypeCombo->currentText();

    if (currentSyncType == "Mutex") {
        loadResBtn->setVisible(false);
        statusLabel->setText("Mutex seleccionado - Solo necesitas cargar acciones para iniciar la simulación.");
    } else {
        loadResBtn->setVisible(true);
        statusLabel->setText("Semáforo seleccionado - Necesitas cargar recursos y acciones para iniciar la simulación.");
    }

    updateInfoDisplay();
}

void SynchronizationSimulatorWidget::updateInfoDisplay()
{
    QString info;
    info += QString("=== %1 ===\n").arg(currentSyncType.toUpper());
    info += QString("Recursos=%1, Acciones=%2\n").arg(resources.size()).arg(actions.size());
    
    if (currentSyncType == "Mutex") {
        info += "Solo un proceso por recurso\nRecursos opcionales\n";
    } else {
        info += "Múltiples procesos por contador\nREAD--, WRITE++\n";
    }
    
    if (!resources.empty()) {
        info += "Recursos:\n";
        for (const auto& resource : resources) {
            info += QString("- %1: %2\n").arg(resource.name).arg(resource.count);
        }
    }
    
    infoDisplay->setPlainText(info);
}

void SynchronizationSimulatorWidget::loadResourcesFromDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Cargar Recursos", "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;
    
    resources = loadResources(fileName);
    updateInfoDisplay();
    statusLabel->setText(QString("Cargados %1 recursos desde archivo.").arg(resources.size()));
    QMessageBox::information(this, "Recursos cargados", QString("Se cargaron %1 recursos.").arg(resources.size()));
}

void SynchronizationSimulatorWidget::loadActionsFromDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Cargar Acciones", "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;
    
    actions = loadActions(fileName);
    updateInfoDisplay();
    statusLabel->setText(QString("Cargadas %1 acciones desde archivo.").arg(actions.size()));
    QMessageBox::information(this, "Acciones cargadas", QString("Se cargaron %1 acciones.").arg(actions.size()));
}

void SynchronizationSimulatorWidget::runSynchronization(const QString &mechanism)
{
    if (actions.empty()) {
        QMessageBox::warning(this, "Datos Faltantes", "Por favor carga acciones antes de ejecutar la simulación.");
        return;
    }
    
    if (mechanism == "Semaphore" && resources.empty()) {
        QMessageBox::warning(this, "Recursos Faltantes", "Por favor carga recursos para la simulación de semáforos.");
        return;
    }
    
    if (processes.empty()) {
        std::set<QString> uniquePids;
        for (const auto& action : actions) {
            uniquePids.insert(action.pid);
        }
        
        int i = 0;
        for (const QString& pid : uniquePids) {
            Process proc;
            proc.pid = pid;
            proc.burst_time = 1;
            proc.arrival_time = 0;
            proc.priority = 1;
            proc.color = QColor(colorPalette[i % colorPalette.size()]);
            processes.push_back(proc);
            processColors[pid] = proc.color;
            i++;
        }
    }
    
    delete syncMechanism;
    if (mechanism == "Mutex Lock")
        syncMechanism = new MutexLock(resources);
    else if (mechanism == "Semaphore")
        syncMechanism = new Semaphore(resources);

    currentEvents = SynchronizationSimulator::simulateSynchronization(processes, resources, actions, syncMechanism);
    
    maxCycles = 0;
    int minCycles = INT_MAX;
    for (const auto& event : currentEvents) {
        maxCycles = std::max(maxCycles, event.cycle);
        minCycles = std::min(minCycles, event.cycle);
    }
    
    syncTable->setRowCount(currentEvents.size());
    for (int i = 0; i < currentEvents.size(); ++i) {
        const auto &event = currentEvents[i];
        syncTable->setItem(i, 0, new QTableWidgetItem(event.pid));
        syncTable->setItem(i, 1, new QTableWidgetItem(event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING"));
        syncTable->setItem(i, 2, new QTableWidgetItem(event.resource));
        syncTable->setItem(i, 3, new QTableWidgetItem(event.action_type));
        syncTable->setItem(i, 4, new QTableWidgetItem(QString::number(event.cycle)));
        
        QColor rowColor = (event.state == ProcessState::ACCESSED) ? QColor("#d4edda") : QColor("#f8d7da");
        for (int j = 0; j < 5; ++j) {
            if (syncTable->item(i, j)) {
                syncTable->item(i, j)->setBackground(rowColor);
            }
        }
    }
    
    setupEmptyTimeline();
    
    currentAnimationCycle = 0;
    animationTimer->start(1500);
    
    statusLabel->setText(QString(" Simulación iniciada. %1 eventos en ciclos 0-%2.")
                        .arg(currentEvents.size()).arg(maxCycles));
}

void SynchronizationSimulatorWidget::setupEmptyTimeline()
{
    if (currentEvents.empty()) return;
    
    auto children = simulationArea->findChildren<QWidget*>();
    for (auto child : children) {
        child->deleteLater();
    }
    
    int minCycle = 0;
    int maxCycle = 0;
    for (const auto& event : currentEvents) {
        maxCycle = std::max(maxCycle, event.cycle);
    }
    
    std::set<QString> uniqueProcesses;
    for (const auto& event : currentEvents) {
        uniqueProcesses.insert(event.pid);
    }
    
    int leftMargin = 80;
    int rightMargin = 50;
    int bottomMargin = 80;
    int cycleWidth = 120;
    int topMargin = 80;
    int processHeight = 80;
    int axisHeight = 50;

    int numProcesses = uniqueProcesses.size();
    
    int totalWidth = leftMargin + (maxCycle + 2) * cycleWidth + rightMargin;
    int totalHeight = topMargin + numProcesses * processHeight + axisHeight + bottomMargin + 40;

    simulationArea->setFixedSize(totalWidth, totalHeight);
    QLabel* titleLabel = new QLabel("Timeline de Sincronización", simulationArea);
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "color: #2c3e50; "
        "background: transparent;"
        "border: none;" 
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 10, totalWidth, 30);
    titleLabel->show();

    int numBlocksMax = 0;
    for (int cycle = 0; cycle <= maxCycle; ++cycle) {
        int count = 0;
        for (const auto& event : currentEvents) {
            if (event.cycle == cycle) count++;
        }
        numBlocksMax = std::max(numBlocksMax, count);
    }
    int blockHeight = 60;
    int blockSpacing = 18;
    int axisY = topMargin + numBlocksMax * (blockHeight + blockSpacing) + 20;

    QLabel* axisLine = new QLabel(simulationArea);
    axisLine->setStyleSheet("background: #2c3e50;");
    axisLine->setGeometry(leftMargin, axisY, (maxCycle + 1) * cycleWidth, 3);
    axisLine->show();

    simulationArea->setProperty("axisY", axisY);

    for (int cycle = 0; cycle <= maxCycle; ++cycle) {
        int xPos = leftMargin + cycle * cycleWidth;

        QLabel* tick = new QLabel(simulationArea);
        tick->setStyleSheet("background: #2c3e50;");
        tick->setGeometry(xPos, axisY, 2, 10);
        tick->show();

        QLabel* numberLabel = new QLabel(QString::number(cycle), simulationArea);
        numberLabel->setStyleSheet(
            "color: #2c3e50; "
            "font-weight: bold; "
            "font-size: 14px; "
            "background: white; "
            "border: 1px solid #bdc3c7; "
            "border-radius: 4px; "
            "padding: 2px;"
        );
        numberLabel->setAlignment(Qt::AlignCenter);
        numberLabel->setGeometry(xPos - 20, axisY + 15, 40, 20);
        numberLabel->show();
    }
    
    std::vector<QString> processList(uniqueProcesses.begin(), uniqueProcesses.end());
    std::sort(processList.begin(), processList.end());
        
    QLabel* waitingInfo = new QLabel(simulationArea);
    waitingInfo->setObjectName("waitingInfo");
    waitingInfo->setText("<b>Estado:</b> Preparando animación...");
    waitingInfo->setStyleSheet("background:#fff3cd;border:1px solid #ffc107;border-radius:4px;padding:5px;font-size:11px;");
    waitingInfo->setWordWrap(true);
    int infoWidth = std::min(800, totalWidth - 40);
    waitingInfo->setGeometry(80, axisY + 40, totalWidth - 160, 40);
    waitingInfo->show();
    
    simulationArea->setProperty("minCycle", minCycle);
    simulationArea->setProperty("maxCycle", maxCycle);
    simulationArea->setProperty("cycleWidth", cycleWidth);
    simulationArea->setProperty("leftMargin", leftMargin);
    simulationArea->setProperty("topMargin", topMargin);
    simulationArea->setProperty("processHeight", processHeight);
    simulationArea->setProperty("processList", QStringList(processList.begin(), processList.end()));
    
    simulationArea->update();
}

void SynchronizationSimulatorWidget::nextAnimationStep()
{
    cycleLabel->setText(QString("Ciclo: %1").arg(currentAnimationCycle));

    int minCycle = simulationArea->property("minCycle").toInt();
    int maxCycle = simulationArea->property("maxCycle").toInt();
    int cycleWidth = simulationArea->property("cycleWidth").toInt();
    int leftMargin = simulationArea->property("leftMargin").toInt();
    int topMargin = simulationArea->property("topMargin").toInt();
    int processHeight = simulationArea->property("processHeight").toInt();

    int blockWidth = 110; 

    QStringList currentCycleInfo;
    int accessedCount = 0;
    int waitingCount = 0;

    int blockIndex = 0;
    int blockSpacing = 18;
    int blockHeight = 60;
    int numBlocksMax = 0;
    // Calcula el máximo de bloques en un ciclo
    for (int cycle = 0; cycle <= maxCycle; ++cycle) {
        int count = 0;
        for (const auto& event : currentEvents) {
            if (event.cycle == cycle) count++;
        }
        numBlocksMax = std::max(numBlocksMax, count);
    }
    int axisY = topMargin + numBlocksMax * (blockHeight + blockSpacing) + 20; // 20px extra de margen

    for (const auto& event : currentEvents) {
        if (event.cycle == currentAnimationCycle) {
            int xPos = leftMargin + currentAnimationCycle * cycleWidth + (cycleWidth - blockWidth) / 2;
            int yPos = topMargin + blockIndex * (blockHeight + blockSpacing); // Empieza debajo de la barra

            QLabel* eventBlock = new QLabel(simulationArea);
            eventBlock->setObjectName(QString("eventBlock_c%1_idx%2").arg(currentAnimationCycle).arg(blockIndex));
            QString eventText = QString(
                "<div style='text-align:center;'>"
                "<span style='font-size:18px;font-weight:bold;'>%1</span><br>"
                "<span style='font-size:12px;'>%2</span><br>"
                "<span style='font-size:11px;'>%3</span>"
                "</div>"
            ).arg(event.pid).arg(event.action_type).arg(event.resource);
            eventBlock->setText(eventText);
            eventBlock->setAlignment(Qt::AlignCenter);
            eventBlock->setWordWrap(true);

            QString bgColor, borderColor, textColor;
            if (event.state == ProcessState::ACCESSED) {
                bgColor = "#6ee7b7";
                borderColor = "#10b981";
                textColor = "#134e4a";
                accessedCount++;
                currentCycleInfo.append(QString("%1:✓%2(%3)").arg(event.pid, event.resource, event.action_type));
            } else {
                bgColor = "#fdba74";
                borderColor = "#ea580c";
                textColor = "#7c2d12";
                waitingCount++;
                currentCycleInfo.append(QString("%1:⏳%2(%3)").arg(event.pid, event.resource, event.action_type));
            }

            eventBlock->setStyleSheet(QString(
                "background: %1;"
                "border: 2px solid %2;"
                "border-radius: 12px;"
                "font-size: 13px;"
                "color: %3;"
                "padding: 4px 2px 4px 2px;"
                "min-width: %4px; min-height: %5px;"
                "max-width: %4px; max-height: %5px;"
            ).arg(bgColor, borderColor, textColor).arg(blockWidth).arg(blockHeight));

            QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(eventBlock);
            shadow->setBlurRadius(8);
            shadow->setColor(QColor(0,0,0,35));
            shadow->setOffset(0, 3);
            eventBlock->setGraphicsEffect(shadow);

            eventBlock->setGeometry(xPos, yPos, blockWidth, blockHeight);
            eventBlock->show();

            blockIndex++;
        }
    }
    
    QLabel* waitingInfo = simulationArea->findChild<QLabel*>("waitingInfo");
    if (waitingInfo) {
        QString infoText = QString("<b>Ciclo %1:</b> ").arg(currentAnimationCycle);
        infoText += QString("Accedidos: %1 | Esperando: %2").arg(accessedCount).arg(waitingCount);
        if (!currentCycleInfo.isEmpty()) {
            infoText += QString(" | %1").arg(currentCycleInfo.join(" • "));
        } else {
            infoText += " | No hay eventos en este ciclo";
        }
        waitingInfo->setText(infoText);
    }
    
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(simulationArea->parentWidget());
    if (scrollArea && currentAnimationCycle > 0) {
        int scrollToX = leftMargin + currentAnimationCycle * cycleWidth - scrollArea->width() / 2;
        scrollToX = std::max(0, std::min(scrollToX, simulationArea->width() - scrollArea->width()));
        scrollArea->horizontalScrollBar()->setValue(scrollToX);
    }
    
    currentAnimationCycle++;
    
    if (currentAnimationCycle > maxCycle + 1) {
        animationTimer->stop();
        cycleLabel->setText(QString("Simulación Completada - Ciclos: 0 a %1").arg(maxCycle));
        statusLabel->setText("Animación completada. Todos los eventos han sido procesados.");
        
        if (waitingInfo) {
            int totalEvents = currentEvents.size();
            int totalAccessed = 0;
            int totalWaiting = 0;
            
            for (const auto& event : currentEvents) {
                if (event.state == ProcessState::ACCESSED) totalAccessed++;
                else totalWaiting++;
            }
            
            waitingInfo->setGeometry(80, simulationArea->property("axisY").toInt() + 40, simulationArea->width() - 160, 40);
            waitingInfo->setText(QString(
                "<b>Simulación Completada:</b> %1 eventos procesados | "
                " %2 accedidos | %3 esperando | "
                "<small>Use scroll horizontal/vertical para navegar por toda la simulación</small>"
            ).arg(totalEvents).arg(totalAccessed).arg(totalWaiting));
        }
    }
}

void SynchronizationSimulatorWidget::clearAll()
{
    processes.clear();
    resources.clear();
    actions.clear();
    currentEvents.clear();
    processColors.clear();
    
    syncTable->setRowCount(0);
    
    QLayout* oldLayout = simulationArea->layout();
    if (oldLayout) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }
    
    auto children = simulationArea->findChildren<QWidget*>();
    for (auto child : children) {
        child->deleteLater();
    }
    
    infoDisplay->clear();
    cycleLabel->setText("Ciclo Actual: 0");
    
    currentAnimationCycle = 0;
    maxCycles = 0;
    
    if (animationTimer->isActive()) {
        animationTimer->stop();
    }
    
    delete syncMechanism;
    syncMechanism = nullptr;
    
    statusLabel->setText("Todos los datos han sido limpiados. Listo para cargar nuevos datos de simulación.");
    updateInfoDisplay();
}

void SynchronizationSimulatorWidget::showInfo()
{
    updateInfoDisplay();
    QMessageBox::information(this, "Información del Sistema", 
        QString("<html>"
               "<h2 style='color:#2c3e50; text-align:center; margin:5px;'>Configuración Actual</h2>"
               "<p style='font-size:14px;'><b>Tipo de Sincronización:</b> %1<br>"
               "<b>Recursos Cargados:</b> %2<br>"
               "<b>Acciones Cargadas:</b> %3<br>"
               "<b>Eventos de Simulación:</b> %4<br>"
               "<b>Ciclos Máximos:</b> %5</p>"
               "<h3 style='color:#2c3e50; margin:10px 0;'>Estados de Ejecución:</h3>"
               "<div style='font-size:16px; margin:10px 0;'>"
               "<span style='background-color:#27ae60; color:white; padding:5px 10px; border-radius:5px; font-weight:bold; margin-right:10px;'>ACCESSED</span>"
               "<span style='vertical-align:middle;'> Acceso concedido al recurso</span>"
               "</div>"
               "<div style='font-size:16px; margin:10px 0;'>"
               "<span style='background-color:#e67e22; color:white; padding:5px 10px; border-radius:5px; font-weight:bold; margin-right:10px;'>WAITING</span>"
               "<span style='vertical-align:middle;'> Esperando acceso al recurso</span>"
               "</div>"
               
               "<h3 style='color:#2c3e50; margin:15px 0 10px 0;'>Visualización Timeline:</h3>"
               "<ul style='font-size:16px; margin:0; padding-left:20px;'>"
               "<li style='margin:5px 0;'><b>Formato:</b> Timeline horizontal con eje X</li>"
               "<li style='margin:5px 0;'><b>Navegación:</b> Use scroll horizontal/vertical</li>"
               "<li style='margin:5px 0;'><b>Ciclos:</b> Numerados en la parte inferior</li>"
               "</ul>"
               "</html>")
        .arg(currentSyncType)
        .arg(resources.size())
        .arg(actions.size())
        .arg(currentEvents.size())
        .arg(maxCycles));
}