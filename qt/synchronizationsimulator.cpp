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

QPushButton* SynchronizationSimulatorWidget::createButton(const QString &text, const QString &color)
{
    QPushButton *btn = new QPushButton(text);
    btn->setMinimumSize(120, 35);
    btn->setFont(QFont("Arial", 11, QFont::Bold));
    btn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: white; border-radius: 6px; padding: 6px; font-size: 11px; }"
        "QPushButton:hover { background-color: %2; }"
    ).arg(color).arg(QColor(color).darker(110).name()));
    return btn;
}

void SynchronizationSimulatorWidget::setupUI()
{
    setWindowTitle("Synchronization Simulator");
    setFixedSize(1500, 900);
    setStyleSheet("background-color: #f8f9fa;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    // Header - MÁS COMPACTO
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

    // Control Panel - MÁS COMPACTO
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    QLabel *typeLabel = new QLabel("Tipo:");
    typeLabel->setFont(QFont("Arial", 10, QFont::Bold));
    
    syncTypeCombo = new QComboBox();
    syncTypeCombo->addItems({"Mutex", "Semaphore"});
    syncTypeCombo->setStyleSheet("QComboBox { padding: 4px; font-size: 10px; border-radius: 4px; }");
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

    // Status and Cycle Counter - MÁS COMPACTO
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

    // Main content with splitter - OPTIMIZADO
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Left panel - MÁS COMPACTO
    QWidget *leftPanel = new QWidget();
    leftPanel->setMaximumWidth(350);
    leftPanel->setMinimumWidth(300);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(4);
    
    // System info - MÁS PEQUEÑO
    QGroupBox *infoGroup = new QGroupBox("Info Sistema");
    infoGroup->setMaximumHeight(80);
    QVBoxLayout *infoGroupLayout = new QVBoxLayout(infoGroup);
    infoGroupLayout->setContentsMargins(4, 4, 4, 4);
    infoDisplay = new QTextEdit();
    infoDisplay->setMaximumHeight(60);
    infoDisplay->setStyleSheet(
        "background-color: #e8f4fd; "
        "border: 1px solid #2196f3; "
        "border-radius: 4px; "
        "padding: 4px; "
        "font-family: monospace; "
        "font-size: 9px;"
    );
    infoDisplay->setReadOnly(true);
    infoGroupLayout->addWidget(infoDisplay);
    leftLayout->addWidget(infoGroup);
    
    // Events table - MÁS COMPACTO
    QGroupBox *eventsGroup = new QGroupBox("Eventos de Simulación");
    QVBoxLayout *eventsLayout = new QVBoxLayout(eventsGroup);
    eventsLayout->setContentsMargins(4, 4, 4, 4);
    syncTable = new QTableWidget();
    syncTable->setColumnCount(5);
    syncTable->setHorizontalHeaderLabels({"Proceso", "Estado", "Recurso", "Acción", "Ciclo"});
    syncTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #ffffff;"
        "  border: 1px solid #c4dafa;"
        "  border-radius: 4px;"
        "  font-size: 9px;"
        "  selection-background-color: #e3f0fc;"
        "}"
        "QHeaderView::section {"
        "  background-color: #6fa8dc;"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 9px;"
        "  border: none;"
        "  padding: 2px;"
        "}"
        "QTableWidget::item {"
        "  padding: 1px;"
        "}"
    );
    syncTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    syncTable->verticalHeader()->setVisible(false);
    syncTable->setShowGrid(false);
    syncTable->setAlternatingRowColors(true);
    syncTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    syncTable->setMaximumHeight(200);
    eventsLayout->addWidget(syncTable);
    leftLayout->addWidget(eventsGroup);
    
    // Right panel - ÁREA DE TIMELINE PRINCIPAL
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(4);
    
    // Timeline visualization area - TIMELINE HORIZONTAL
    QGroupBox *timelineGroup = new QGroupBox("Timeline Horizontal de Sincronización - Scroll horizontal dinámico");
    QVBoxLayout *timelineLayout = new QVBoxLayout(timelineGroup);
    timelineLayout->setContentsMargins(4, 4, 4, 4);
    
    // NUEVA IMPLEMENTACIÓN: Widget personalizado para timeline horizontal
    simulationArea = new QWidget();
    simulationArea->setMinimumHeight(500);
    simulationArea->setStyleSheet("background: #fff; border: 1px solid #e3f0fc;");
    
    // CRUCIAL: ScrollArea con scroll horizontal
    QScrollArea *timelineScroll = new QScrollArea();
    timelineScroll->setWidget(simulationArea);
    timelineScroll->setWidgetResizable(false); // IMPORTANTE para scroll horizontal
    timelineScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineScroll->setMinimumHeight(520);
    
    timelineLayout->addWidget(timelineScroll);
    rightLayout->addWidget(timelineGroup);
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({300, 1200});

    // Assemble main layout
    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(controlLayout);
    mainLayout->addLayout(statusLayout);
    mainLayout->addWidget(mainSplitter);

    // Connections
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
    
    statusLabel->setText(QString("🚀 Simulación iniciada. %1 eventos en ciclos 0-%2.")
                        .arg(currentEvents.size()).arg(maxCycles));
}

void SynchronizationSimulatorWidget::setupEmptyTimeline()
{
    if (currentEvents.empty()) return;
    
    // Limpiar widgets existentes
    auto children = simulationArea->findChildren<QWidget*>();
    for (auto child : children) {
        child->deleteLater();
    }
    
    // Encontrar el rango real de ciclos - EMPEZAR DESDE 0
    int minCycle = 0; // FORZAR inicio en 0
    int maxCycle = 0;
    for (const auto& event : currentEvents) {
        maxCycle = std::max(maxCycle, event.cycle);
    }
    
    // Obtener lista única de procesos
    std::set<QString> uniqueProcesses;
    for (const auto& event : currentEvents) {
        uniqueProcesses.insert(event.pid);
    }
    
    // Calcular dimensiones para timeline horizontal
    int leftMargin = 80;   // Espacio para etiquetas de procesos
    int rightMargin = 50;
    int topMargin = 60;    // Espacio para números del eje X
    int bottomMargin = 40;
    int cycleWidth = 40;   // Ancho por cada unidad de tiempo
    int processHeight = 50; // Altura para cada fila de proceso
    int axisHeight = 30;   // Altura del eje X
    
    int numProcesses = uniqueProcesses.size();
    
    // Calcular tamaño total
    int totalWidth = leftMargin + (maxCycle + 2) * cycleWidth + rightMargin;
    int totalHeight = topMargin + numProcesses * processHeight + axisHeight + bottomMargin;
    
    qDebug() << "Timeline horizontal: Ciclos 0 a" << maxCycle 
             << "| Procesos:" << numProcesses
             << "| Tamaño:" << totalWidth << "x" << totalHeight;
    
    // Redimensionar simulationArea para scroll horizontal
    simulationArea->setFixedSize(totalWidth, totalHeight);
    simulationArea->setStyleSheet("background: white; border: 1px solid #ddd;");
    
    // Crear título
    QLabel* titleLabel = new QLabel("Timeline de Sincronización", simulationArea);
    titleLabel->setStyleSheet(
        "font-size: 14px; "
        "font-weight: bold; "
        "color: #2c3e50; "
        "background: transparent;"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 5, totalWidth, 25);
    titleLabel->show();
    
    // Crear eje X con números
    int axisY = topMargin + numProcesses * processHeight;
    
    // Línea principal del eje X
    QLabel* axisLine = new QLabel(simulationArea);
    axisLine->setStyleSheet("background: #2c3e50;");
    axisLine->setGeometry(leftMargin, axisY, (maxCycle + 1) * cycleWidth, 2);
    axisLine->show();
    
    // Números y marcas del eje X
    for (int cycle = 0; cycle <= maxCycle; ++cycle) {
        int xPos = leftMargin + cycle * cycleWidth;
        
        // Marca vertical
        QLabel* tick = new QLabel(simulationArea);
        tick->setStyleSheet("background: #2c3e50;");
        tick->setGeometry(xPos, axisY, 1, 8);
        tick->show();
        
        // Número
        QLabel* numberLabel = new QLabel(QString::number(cycle), simulationArea);
        numberLabel->setStyleSheet(
            "color: #2c3e50; "
            "font-weight: bold; "
            "font-size: 12px; "
            "background: transparent;"
        );
        numberLabel->setAlignment(Qt::AlignCenter);
        numberLabel->setGeometry(xPos - 15, axisY + 10, 30, 20);
        numberLabel->show();
    }
    
    // Etiquetas de procesos en el lado izquierdo
    std::vector<QString> processList(uniqueProcesses.begin(), uniqueProcesses.end());
    std::sort(processList.begin(), processList.end()); // Ordenar para consistencia
    
    for (int i = 0; i < processList.size(); ++i) {
        QString pid = processList[i];
        int yPos = topMargin + i * processHeight;
        
        // Etiqueta del proceso
        QLabel* processLabel = new QLabel(pid, simulationArea);
        processLabel->setStyleSheet(
            "background: #ecf0f1; "
            "border: 2px solid #bdc3c7; "
            "border-radius: 5px; "
            "color: #2c3e50; "
            "font-weight: bold; "
            "font-size: 12px;"
        );
        processLabel->setAlignment(Qt::AlignCenter);
        processLabel->setGeometry(10, yPos + 10, 60, 30);
        processLabel->show();
        
        // Línea horizontal para el proceso
        QLabel* processLine = new QLabel(simulationArea);
        processLine->setStyleSheet("background: #ecf0f1;");
        processLine->setGeometry(leftMargin, yPos + 25, (maxCycle + 1) * cycleWidth, 1);
        processLine->show();
    }
    
    // Crear leyenda
    QLabel* legend = new QLabel(simulationArea);
    legend->setText(
        "<b>Leyenda:</b> "
        "<span style='background: #27ae60; color: white; padding: 3px 8px; border-radius: 3px; margin: 2px;'>ACCESSED</span> "
        "<span style='background: #e67e22; color: white; padding: 3px 8px; border-radius: 3px; margin: 2px;'>WAITING</span> "
        "| Timeline horizontal con scroll"
    );
    legend->setStyleSheet(
        "background: #ecf0f1; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 6px; "
        "padding: 8px; "
        "font-size: 11px;"
    );
    legend->setGeometry(leftMargin + 50, 30, 600, 25);
    legend->show();
    
    // Área de información de estado
    QLabel* waitingInfo = new QLabel(simulationArea);
    waitingInfo->setObjectName("waitingInfo");
    waitingInfo->setText("<b>Estado de la Simulación:</b> Preparando animación...");
    waitingInfo->setStyleSheet(
        "background: #fff3cd; "
        "border: 2px solid #ffc107; "
        "border-radius: 6px; "
        "padding: 8px; "
        "font-size: 10px;"
    );
    waitingInfo->setWordWrap(true);
    waitingInfo->setGeometry(10, totalHeight - 35, totalWidth - 20, 30);
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

void SynchronizationSimulatorWidget::nextAnimationStep()
{
    // Actualizar contador de ciclo
    cycleLabel->setText(QString("Ciclo Actual: %1").arg(currentAnimationCycle));
    
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
    
    // Agregar bloques para eventos en el ciclo actual
    for (const auto& event : currentEvents) {
        if (event.cycle == currentAnimationCycle) {
            // Calcular posición horizontal (eje X)
            int xPos = leftMargin + currentAnimationCycle * cycleWidth;
            
            // Calcular posición vertical (fila del proceso)
            int processRow = processToRow[event.pid];
            int yPos = topMargin + processRow * processHeight + 10; // +10 para centrar
            
            // Crear bloque del evento
            QLabel* eventBlock = new QLabel(simulationArea);
            eventBlock->setObjectName(QString("eventBlock_c%1_p%2").arg(currentAnimationCycle).arg(event.pid));
            
            // Texto del evento
            QString eventText = QString("%1\n%2").arg(event.action_type.toUpper(), event.resource);
            eventBlock->setText(eventText);
            eventBlock->setAlignment(Qt::AlignCenter);
            
            // Colores según estado
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
                "border-radius: 5px; "
                "font-weight: bold; "
                "font-size: 9px; "
                "color: %3;"
            ).arg(bgColor, borderColor, textColor));
            
            // Posicionamiento del bloque
            int blockWidth = cycleWidth - 4; // Dejar pequeño margen
            int blockHeight = 30;
            eventBlock->setGeometry(xPos + 2, yPos, blockWidth, blockHeight);
            eventBlock->show();
            
            qDebug() << "Bloque horizontal:" << event.pid << event.action_type 
                     << (event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING") 
                     << "en ciclo" << currentAnimationCycle << "posición" << xPos << yPos;
        }
    }
    
    // Actualizar información de estado
    QLabel* waitingInfo = simulationArea->findChild<QLabel*>("waitingInfo");
    if (waitingInfo) {
        QString infoText = QString("<b>Ciclo %1:</b> ").arg(currentAnimationCycle);
        infoText += QString("Accedidos: %1 | ⏳ Esperando: %2").arg(accessedCount).arg(waitingCount);
        if (!currentCycleInfo.isEmpty()) {
            infoText += QString(" | %1").arg(currentCycleInfo.join(" • "));
        } else {
            infoText += " | No hay eventos en este ciclo";
        }
        waitingInfo->setText(infoText);
    }
    
    // Auto-scroll para seguir la animación
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(simulationArea->parentWidget());
    if (scrollArea && currentAnimationCycle > 0) {
        int scrollToX = leftMargin + currentAnimationCycle * cycleWidth - scrollArea->width() / 2;
        scrollToX = std::max(0, scrollToX);
        scrollArea->horizontalScrollBar()->setValue(scrollToX);
    }
    
    // Avanzar al siguiente ciclo
    currentAnimationCycle++;
    
    // Verificar si la animación ha terminado
    if (currentAnimationCycle > maxCycle + 1) {
        animationTimer->stop();
        cycleLabel->setText(QString("Simulación Completada - Ciclos: 0 a %1").arg(maxCycle));
        statusLabel->setText("Animación completada. Todos los eventos han sido procesados.");
        
        // Mostrar resumen final
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
                " %2 accedidos | ⏳ %3 esperando | "
                "<small>Use scroll horizontal para navegar</small>"
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
        QString("Configuración Actual:\n\n"
               "Tipo de Sincronización: %1\n"
               "Recursos Cargados: %2\n"
               "Acciones Cargadas: %3\n"
               "Eventos de Simulación: %4\n"
               "Ciclos Máximos: %5\n\n"
               "Estados: ACCESSED (verde), WAITING (naranja)\n"
               "Formato: Timeline horizontal con eje X\n"
               "Scroll horizontal disponible")
        .arg(currentSyncType)
        .arg(resources.size())
        .arg(actions.size())
        .arg(currentEvents.size())
        .arg(maxCycles));
}