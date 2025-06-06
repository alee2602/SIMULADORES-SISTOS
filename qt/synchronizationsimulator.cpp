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
    setFixedSize(1400, 900); // REDUCIDO de 1600x1000 a 1400x900
    setStyleSheet("background-color: #f8f9fa;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6); // Reducido de 8 a 6

    // Header - MÁS COMPACTO
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backBtn = createButton("← Back to Menu", "#6c757d");
    QLabel *title = new QLabel("SIMULADOR DE MECANISMOS DE SINCRONIZACIÓN");
    title->setFont(QFont("Arial", 16, QFont::Bold)); // Reducido de 18 a 16
    title->setStyleSheet(
        "color: #2c3e50;"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #c4e5fb, stop:1 #e3f0fc);"
        "border-radius: 8px;" // Reducido de 10 a 8
        "padding: 8px;" // Reducido de 12 a 8
        "margin: 2px;" // Reducido de 4 a 2
    );
    title->setAlignment(Qt::AlignCenter);
    title->setMaximumHeight(40); // Reducido de 50 a 40
    
    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(title, 1);
    headerLayout->addStretch();

    // Control Panel - MÁS COMPACTO
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    QLabel *typeLabel = new QLabel("Tipo:");
    typeLabel->setFont(QFont("Arial", 10, QFont::Bold)); // Reducido de 11 a 10
    
    syncTypeCombo = new QComboBox();
    syncTypeCombo->addItems({"Mutex", "Semaphore"});
    syncTypeCombo->setStyleSheet(
        "QComboBox { "
        "  padding: 4px; " // Reducido de 6 a 4
        "  font-size: 10px; " // Reducido de 11 a 10
        "  border-radius: 4px; " // Reducido de 5 a 4
        "  border: 1px solid #ced4da; "
        "  background: white; "
        "}"
    );
    syncTypeCombo->setMaximumHeight(30); // Reducido de 35 a 30

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

    // Status and Cycle Counter - MÁS COMPACTO
    QHBoxLayout *statusLayout = new QHBoxLayout();
    
    statusLabel = new QLabel("Mutex seleccionado - Solo necesitas cargar acciones para iniciar la simulación.");
    statusLabel->setStyleSheet(
        "color: #495057; "
        "background: #e9ecef; "
        "border: 1px solid #ced4da; " // Reducido de 2 a 1
        "border-radius: 4px; " // Reducido de 6 a 4
        "padding: 6px; " // Reducido de 10 a 6
        "font-size: 10px;" // Reducido de 12 a 10
    );
    statusLabel->setMaximumHeight(25); // Reducido de 35 a 25
    statusLabel->setWordWrap(true);
    
    cycleLabel = new QLabel("Ciclo Actual: 0");
    cycleLabel->setFont(QFont("Arial", 12, QFont::Bold)); // Reducido de 14 a 12
    cycleLabel->setStyleSheet(
        "color: #2c3e50; "
        "background: white; "
        "border: 2px solid #3498db; " // Reducido de 3 a 2
        "border-radius: 6px; " // Reducido de 8 a 6
        "padding: 6px; " // Reducido de 10 a 6
    );
    cycleLabel->setAlignment(Qt::AlignCenter);
    cycleLabel->setMinimumWidth(120); // Reducido de 140 a 120
    cycleLabel->setMaximumHeight(30); // Reducido de 40 a 30
    
    statusLayout->addWidget(statusLabel, 3);
    statusLayout->addWidget(cycleLabel, 1);

    // Main content with splitter - PROPORCIONES MEJORADAS
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setHandleWidth(8); // Reducido de 10 a 8
    
    // Left panel - MÁS COMPACTO
    QWidget *leftPanel = new QWidget();
    leftPanel->setMaximumWidth(320); // Reducido de 400 a 320
    leftPanel->setMinimumWidth(300); // Reducido de 350 a 300
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(6); // Reducido de 10 a 6
    
    // System info - MÁS COMPACTO
    QGroupBox *infoGroup = new QGroupBox("Información del Sistema");
    infoGroup->setFont(QFont("Arial", 10, QFont::Bold)); // Reducido de 11 a 10
    infoGroup->setMaximumHeight(100); // Reducido de 120 a 100
    infoGroup->setStyleSheet(
        "QGroupBox { "
        "  font-weight: bold; "
        "  border: 1px solid #bdc3c7; " // Reducido de 2 a 1
        "  border-radius: 6px; " // Reducido de 8 a 6
        "  margin-top: 8px; " // Reducido de 10 a 8
        "  background: #ecf0f1; "
        "} "
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 8px; " // Reducido de 10 a 8
        "  padding: 0 6px 0 6px; " // Reducido de 8 a 6
        "  background: white; "
        "  border-radius: 3px; " // Reducido de 4 a 3
        "}"
    );
    QVBoxLayout *infoGroupLayout = new QVBoxLayout(infoGroup);
    infoGroupLayout->setContentsMargins(6, 12, 6, 6); // Reducido
    infoDisplay = new QTextEdit();
    infoDisplay->setMaximumHeight(70); // Reducido de 90 a 70
    infoDisplay->setStyleSheet(
        "background-color: #e8f4fd; "
        "border: 1px solid #2196f3; " // Reducido de 2 a 1
        "border-radius: 4px; " // Reducido de 6 a 4
        "padding: 4px; " // Reducido de 8 a 4
        "font-family: 'Consolas', monospace; "
        "font-size: 9px;" // Reducido de 11 a 9
    );
    infoDisplay->setReadOnly(true);
    infoGroupLayout->addWidget(infoDisplay);
    leftLayout->addWidget(infoGroup);
    
    // Events table - MÁS COMPACTO PERO FUNCIONAL
    QGroupBox *eventsGroup = new QGroupBox("Eventos de Simulación");
    eventsGroup->setFont(QFont("Arial", 10, QFont::Bold)); // Reducido de 11 a 10
    eventsGroup->setStyleSheet(
        "QGroupBox { "
        "  font-weight: bold; "
        "  border: 1px solid #bdc3c7; " // Reducido de 2 a 1
        "  border-radius: 6px; " // Reducido de 8 a 6
        "  margin-top: 8px; " // Reducido de 10 a 8
        "  background: #ecf0f1; "
        "} "
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 8px; " // Reducido de 10 a 8
        "  padding: 0 6px 0 6px; " // Reducido de 8 a 6
        "  background: white; "
        "  border-radius: 3px; " // Reducido de 4 a 3
        "}"
    );
    QVBoxLayout *eventsLayout = new QVBoxLayout(eventsGroup);
    eventsLayout->setContentsMargins(6, 12, 6, 6); // Reducido
    
    syncTable = new QTableWidget();
    syncTable->setColumnCount(5);
    syncTable->setHorizontalHeaderLabels({"Proceso", "Estado", "Recurso", "Acción", "Ciclo"});
    
    // TABLA MÁS COMPACTA PERO LEGIBLE
    syncTable->setMinimumHeight(520); // Reducido de 620 a 520
    syncTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #ffffff;"
        "  border: 1px solid #3498db;" // Reducido de 2 a 1
        "  border-radius: 6px;" // Reducido de 8 a 6
        "  font-size: 10px;" // Reducido de 12 a 10
        "  selection-background-color: #e3f0fc;"
        "  gridline-color: #ecf0f1;"
        "  margin: 2px;" // Reducido de 5 a 2
        "}"
        "QHeaderView::section {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 10px;" // Reducido de 12 a 10
        "  border: none;"
        "  padding: 8px;" // Reducido de 12 a 8
        "  border-right: 1px solid #2980b9;"
        "}"
        "QTableWidget::item {"
        "  padding: 6px;" // Reducido de 10 a 6
        "  border-bottom: 1px solid #ecf0f1;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #d4edda;"
        "  color: #155724;"
        "}"
    );
    
    // Configuración de la tabla - MÁS COMPACTA
    syncTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    syncTable->horizontalHeader()->setStretchLastSection(true);
    syncTable->verticalHeader()->setVisible(false);
    syncTable->setShowGrid(true);
    syncTable->setAlternatingRowColors(true);
    syncTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Anchos de columna más compactos
    syncTable->setColumnWidth(0, 60);  // Proceso - Reducido de 80 a 60
    syncTable->setColumnWidth(1, 70);  // Estado - Reducido de 90 a 70
    syncTable->setColumnWidth(2, 60);  // Recurso - Reducido de 80 a 60
    syncTable->setColumnWidth(3, 60);  // Acción - Reducido de 80 a 60
    // La columna 5 (Ciclo) se estirará automáticamente
    
    eventsLayout->addWidget(syncTable);
    leftLayout->addWidget(eventsGroup);
    
    // Right panel - TIMELINE OPTIMIZADO
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(4); // Reducido de 6 a 4
    
    // Timeline visualization area - MÁS COMPACTO
    QGroupBox *timelineGroup = new QGroupBox("Timeline Horizontal de Sincronización");
    timelineGroup->setFont(QFont("Arial", 11, QFont::Bold)); // Reducido de 12 a 11
    timelineGroup->setStyleSheet(
        "QGroupBox { "
        "  font-weight: bold; "
        "  border: 1px solid #2c3e50; " // Reducido de 2 a 1
        "  border-radius: 8px; " // Reducido de 10 a 8
        "  margin-top: 10px; " // Reducido de 12 a 10
        "  background: #ecf0f1; "
        "} "
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 12px; " // Reducido de 15 a 12
        "  padding: 0 8px 0 8px; " // Reducido de 10 a 8
        "  background: white; "
        "  border-radius: 4px; " // Reducido de 6 a 4
        "  color: #2c3e50; "
        "}"
    );
    QVBoxLayout *timelineLayout = new QVBoxLayout(timelineGroup);
    timelineLayout->setContentsMargins(8, 15, 8, 8); // Reducido
    
    // Widget para timeline MÁS COMPACTO
    simulationArea = new QWidget();
    simulationArea->setMinimumHeight(600);  // Aumentado de 480 a 600 para tener más altura
    simulationArea->setStyleSheet(
        "background: #ffffff; "
        "border: 1px solid #3498db; "
        "border-radius: 6px;"
    );
    
    // ScrollArea con mayor altura vertical real
    QScrollArea *timelineScroll = new QScrollArea();
    timelineScroll->setWidget(simulationArea);
    timelineScroll->setWidgetResizable(false);
    timelineScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    timelineScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineScroll->setMinimumHeight(720);  // Mantener este valor, pero arreglar la configuración del layout
    timelineScroll->setMaximumHeight(720);  // Agregar esta línea para forzar la altura exacta

    // Asegurar que el timelineGroup se expande para ocupar el espacio disponible
    timelineGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    timelineLayout->addWidget(timelineScroll);
    rightLayout->addWidget(timelineGroup);
    
    // Add panels to splitter - PROPORCIONES OPTIMIZADAS PARA MENOR ANCHO
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({300, 1100}); // Ajustado para 1400px total

    // Assemble main layout
    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(statusLayout);
    mainLayout->addWidget(mainSplitter);

    // Connections (mantener las mismas conexiones)
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
    
    // Initial state
    onSyncTypeChanged();
}

// Función auxiliar para crear botones MÁS GRANDES y MÁS LARGOS
QPushButton* SynchronizationSimulatorWidget::createButton(const QString &text, const QString &color)
{
    QPushButton *btn = new QPushButton(text);
    btn->setMinimumSize(180, 45); // Aumentado de 150,40 a 180,45 para hacer más largos y anchos
    btn->setFont(QFont("Arial", 14, QFont::Bold)); // Mantenemos el tamaño de fuente grande
    btn->setStyleSheet(QString(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:1 %2); "
        "  color: white; "
        "  border-radius: 8px; " // Aumentado de 6 a 8
        "  padding: 8px; " // Aumentado de 6 a 8
        "  font-size: 11px; " // Aumentado de 9 a 11
        "  border: 2px solid %3; " // Aumentado de 1 a 2
        "  margin: 3px; " // Aumentado de 2 a 3
        "}"
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %4, stop:1 %5); "
        "  margin-top: 2px; " // Aumentado de 1 a 2
        "  margin-bottom: 4px; " // Aumentado de 3 a 4
        "}"
        "QPushButton:pressed { "
        "  background: %6; "
        "  margin-top: 4px; " // Aumentado de 3 a 4
        "  margin-bottom: 2px; " // Aumentado de 1 a 2
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

    // Mostrar/ocultar el botón de recursos según el tipo
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
    
    // Create dummy processes from actions if not loaded
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
    
    // Find max cycles
    maxCycles = 0;
    int minCycles = INT_MAX;
    for (const auto& event : currentEvents) {
        maxCycles = std::max(maxCycles, event.cycle);
        minCycles = std::min(minCycles, event.cycle);
    }
    
    // Update events table
    syncTable->setRowCount(currentEvents.size());
    for (int i = 0; i < currentEvents.size(); ++i) {
        const auto &event = currentEvents[i];
        syncTable->setItem(i, 0, new QTableWidgetItem(event.pid));
        syncTable->setItem(i, 1, new QTableWidgetItem(event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING"));
        syncTable->setItem(i, 2, new QTableWidgetItem(event.resource));
        syncTable->setItem(i, 3, new QTableWidgetItem(event.action_type));
        syncTable->setItem(i, 4, new QTableWidgetItem(QString::number(event.cycle)));
        
        // Color based on state
        QColor rowColor = (event.state == ProcessState::ACCESSED) ? QColor("#d4edda") : QColor("#f8d7da");
        for (int j = 0; j < 5; ++j) {
            if (syncTable->item(i, j)) {
                syncTable->item(i, j)->setBackground(rowColor);
            }
        }
    }
    
    // Setup timeline horizontal
    setupEmptyTimeline();
    
    // Start animation from cycle 0
    currentAnimationCycle = 0;
    animationTimer->start(1500); // 1.5 seconds per cycle
    
    statusLabel->setText(QString(" Simulación iniciada. %1 eventos en ciclos 0-%2.")
                        .arg(currentEvents.size()).arg(maxCycles));
}

// 1. MODIFICAR setupEmptyTimeline() - Ajustar dimensiones y scroll
void SynchronizationSimulatorWidget::setupEmptyTimeline()
{
    if (currentEvents.empty()) return;
    
    // Limpiar widgets existentes
    auto children = simulationArea->findChildren<QWidget*>();
    for (auto child : children) {
        child->deleteLater();
    }
    
    // Encontrar el rango real de ciclos
    int minCycle = 0;
    int maxCycle = 0;
    for (const auto& event : currentEvents) {
        maxCycle = std::max(maxCycle, event.cycle);
    }
    
    // Obtener lista única de procesos
    std::set<QString> uniqueProcesses;
    for (const auto& event : currentEvents) {
        uniqueProcesses.insert(event.pid);
    }
    
    // DIMENSIONES MEJORADAS con MÁS espacio vertical entre procesos
    int leftMargin = 80;      // Espacio para etiquetas
    int rightMargin = 50;     // Margen derecho
    int topMargin = 80;       // Espacio para título y leyenda
    int bottomMargin = 80;    // Aumentado para más espacio
    int cycleWidth = 120;     // Ancho por ciclo
    int processHeight = 80;   // Aumentado de 60 a 80 para más espacio entre procesos
    int axisHeight = 50;      // Aumentado de 40 a 50

    int numProcesses = uniqueProcesses.size();
    
    // Calcular tamaño total con MÁS espacio 
    int totalWidth = leftMargin + (maxCycle + 2) * cycleWidth + rightMargin;
    // Hacer el área más alta
    int totalHeight = topMargin + numProcesses * processHeight + axisHeight + bottomMargin + 40;  // +40 para más espacio
    
    qDebug() << "Timeline COMPACTO: Ciclos 0 a" << maxCycle 
             << "| Procesos:" << numProcesses
             << "| Tamaño TOTAL:" << totalWidth << "x" << totalHeight;

    // Redimensionar simulationArea
    simulationArea->setFixedSize(totalWidth, totalHeight);
    // Título
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
    
    // Eje X
    int axisY = topMargin + numProcesses * processHeight;
    QLabel* axisLine = new QLabel(simulationArea);
    axisLine->setStyleSheet("background: #2c3e50;");
    axisLine->setGeometry(leftMargin, axisY, (maxCycle + 1) * cycleWidth, 3);
    axisLine->show();
    
    // Números del eje X
    for (int cycle = 0; cycle <= maxCycle; ++cycle) {
        int xPos = leftMargin + cycle * cycleWidth;
        
        // Marca vertical
        QLabel* tick = new QLabel(simulationArea);
        tick->setStyleSheet("background: #2c3e50;");
        tick->setGeometry(xPos, axisY, 2, 10);
        tick->show();
        
        // Número del ciclo
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
    
    // Se eliminaron las etiquetas de procesos
    std::vector<QString> processList(uniqueProcesses.begin(), uniqueProcesses.end());
    std::sort(processList.begin(), processList.end());
        
    // Área de información MEJORADA
    QLabel* waitingInfo = new QLabel(simulationArea);
    waitingInfo->setObjectName("waitingInfo");
    waitingInfo->setText("<b>Estado:</b> Preparando animación...");
    waitingInfo->setStyleSheet("background:#fff3cd;border:1px solid #ffc107;border-radius:4px;padding:5px;font-size:11px;");
    waitingInfo->setWordWrap(true);
    int infoWidth = std::min(800, totalWidth - 40);
    waitingInfo->setGeometry(80, totalHeight - 50, totalWidth - 160, 40);
    waitingInfo->show();
    
    // Guardar propiedades para la animación
    simulationArea->setProperty("minCycle", minCycle);
    simulationArea->setProperty("maxCycle", maxCycle);
    simulationArea->setProperty("cycleWidth", cycleWidth);
    simulationArea->setProperty("leftMargin", leftMargin);
    simulationArea->setProperty("topMargin", topMargin);
    simulationArea->setProperty("processHeight", processHeight);
    simulationArea->setProperty("processList", QStringList(processList.begin(), processList.end()));
    
    simulationArea->update();
}

// 2. MODIFICAR nextAnimationStep() - Mejorar el display de bloques
void SynchronizationSimulatorWidget::nextAnimationStep()
{
    // Actualizar contador de ciclo
    cycleLabel->setText(QString("Ciclo: %1").arg(currentAnimationCycle));
    
    // Obtener valores guardados
    int minCycle = simulationArea->property("minCycle").toInt();
    int maxCycle = simulationArea->property("maxCycle").toInt();
    int cycleWidth = simulationArea->property("cycleWidth").toInt();
    int leftMargin = simulationArea->property("leftMargin").toInt();
    int topMargin = simulationArea->property("topMargin").toInt();
    int processHeight = simulationArea->property("processHeight").toInt();
    QStringList processList = simulationArea->property("processList").toStringList();
    
    // Crear mapa de proceso a índice de fila
    std::map<QString, int> processToRow;
    for (int i = 0; i < processList.size(); ++i) {
        processToRow[processList[i]] = i;
    }
    
    // Información del ciclo actual
    QStringList currentCycleInfo;
    int accessedCount = 0;
    int waitingCount = 0;
    
    // Agregar bloques con MENOS ESPACIO VERTICAL entre ellos
    for (const auto& event : currentEvents) {
        if (event.cycle == currentAnimationCycle) {
            // Calcular posición horizontal (eje X)
            int xPos = leftMargin + currentAnimationCycle * cycleWidth;
            
            // Calcular posición vertical (fila del proceso) - SIN ESPACIADO ADICIONAL
            int processRow = processToRow[event.pid];
            int yPos = topMargin + processRow * processHeight + 5; // Reducido margen superior
            
            // Crear bloque del evento 
            QLabel* eventBlock = new QLabel(simulationArea);
            eventBlock->setObjectName(QString("eventBlock_c%1_p%2").arg(currentAnimationCycle).arg(event.pid));
            
            // Formato de texto como en la imagen de ejemplo
            QString eventText = QString("<center><b>%1</b><br>%2<br>%3</center>")
                               .arg(event.pid)          // ID del proceso (P1, P2)
                               .arg(event.action_type)  // WRITE, READ
                               .arg(event.resource);    // R1, etc
            
            eventBlock->setText(eventText);
            eventBlock->setAlignment(Qt::AlignCenter);
            eventBlock->setWordWrap(true);
            
            // Colores según estado (como en la imagen)
            QString bgColor, borderColor, textColor;
            if (event.state == ProcessState::ACCESSED) {
                bgColor = "#27ae60";      // Verde para ACCESSED
                borderColor = "#1e8449";
                textColor = "white";
                accessedCount++;
                currentCycleInfo.append(QString("%1:✓%2(%3)").arg(event.pid, event.resource, event.action_type));
            } else {
                bgColor = "#e67e22";      // Naranja para WAITING
                borderColor = "#d35400";
                textColor = "white";
                waitingCount++;
                currentCycleInfo.append(QString("%1:⏳%2(%3)").arg(event.pid, event.resource, event.action_type));
            }
            
            eventBlock->setStyleSheet(QString(
                "background: %1; "
                "border: 2px solid %2; "
                "border-radius: 8px; " 
                "font-weight: bold; "
                "font-size: 12px; "
                "color: %3; "
                "padding: 4px;"
            ).arg(bgColor, borderColor, textColor));
            
            // Bloques de tamaño ajustado para evitar espacios vacíos
            int blockWidth = cycleWidth - 20;
            int blockHeight = processHeight - 15;  // Un poco menos de margen para hacer bloques más grandes
            eventBlock->setGeometry(xPos + 10, yPos + 5, blockWidth, blockHeight);  // +5 para centrar mejor
            eventBlock->show();
            
            qDebug() << "Bloque compacto:" << event.pid << event.action_type 
                     << (event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING") 
                     << "en ciclo" << currentAnimationCycle << "posición" << xPos << yPos;
        }
    }
    
    // Actualizar información de estado MEJORADA
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
    
    // SCROLL AUTOMÁTICO MEJORADO
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(simulationArea->parentWidget());
    if (scrollArea && currentAnimationCycle > 0) {
        int cycleWidth = simulationArea->property("cycleWidth").toInt();
        int leftMargin = simulationArea->property("leftMargin").toInt();
        int scrollToX = leftMargin + currentAnimationCycle * cycleWidth - scrollArea->width() / 2;
        scrollToX = std::max(0, std::min(scrollToX, simulationArea->width() - scrollArea->width()));
        scrollArea->horizontalScrollBar()->setValue(scrollToX);
        
        qDebug() << "Auto-scroll a X:" << scrollToX << "para ciclo" << currentAnimationCycle;
    }
    
    // Avanzar al siguiente ciclo
    currentAnimationCycle++;
    
    // Verificar si la animación ha terminado
    if (currentAnimationCycle > maxCycle + 1) {
        animationTimer->stop();
        cycleLabel->setText(QString("Simulación Completada - Ciclos: 0 a %1").arg(maxCycle));
        statusLabel->setText("Animación completada. Todos los eventos han sido procesados.");
        
        // Mostrar resumen final MEJORADO
        if (waitingInfo) {
            int totalEvents = currentEvents.size();
            int totalAccessed = 0;
            int totalWaiting = 0;
            
            for (const auto& event : currentEvents) {
                if (event.state == ProcessState::ACCESSED) totalAccessed++;
                else totalWaiting++;
            }
            
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
    
    // Clear simulation area
    QLayout* oldLayout = simulationArea->layout();
    if (oldLayout) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }
    
    // Limpiar widgets hijos manualmente
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