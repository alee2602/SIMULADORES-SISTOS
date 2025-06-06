#include "synchronizer.h"
#include "synchronizationsimulator.h"
#include "loader.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>

SynchronizationSimulatorWidget::SynchronizationSimulatorWidget(QWidget *parent)
    : QWidget(parent), syncMechanism(nullptr)
{
    setupUI();
}

SynchronizationSimulatorWidget::~SynchronizationSimulatorWidget()
{
    delete syncMechanism;
}

void SynchronizationSimulatorWidget::setupUI()
{
    setStyleSheet("background-color: #f8f9fa;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Título
    QLabel *title = new QLabel("SIMULADOR DE MECANISMOS DE SINCRONIZACIÓN");
    title->setFont(QFont("Arial", 15, QFont::Bold));
    title->setStyleSheet(
        "color: #2c3e50;"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #c4e5fb, stop:1 #e3f0fc);"
        "border-radius: 10px;"
        "padding: 8px;"
        "margin: 8px;"
    );
    title->setAlignment(Qt::AlignCenter);
    title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    layout->addWidget(title);

    // Botones
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    auto createStyledButton = [](const QString &text, const QString &color) {
        QPushButton *btn = new QPushButton(text);
        btn->setMinimumSize(180, 40);
        btn->setFont(QFont("Arial", 13, QFont::Bold));
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: black; border-radius: 10px; padding: 8px; font-size: 15px; }"
            "QPushButton:hover { background-color: #e3f0fc; }"
        ).arg(color));
        return btn;
    };

    QPushButton *loadProcBtn = createStyledButton("Cargar Procesos", "#c4dafa");
    QPushButton *loadResBtn = createStyledButton("Cargar Recursos", "#c4e5fb");
    QPushButton *loadActBtn = createStyledButton("Cargar Acciones", "#d4f7fa");
    QPushButton *runMutexBtn = createStyledButton("Simular Mutex Lock", "#b9e6c9");
    QPushButton *runSemBtn = createStyledButton("Simular Semáforo", "#ffe4b5");

    buttonLayout->addWidget(loadProcBtn);
    buttonLayout->addWidget(loadResBtn);
    buttonLayout->addWidget(loadActBtn);
    buttonLayout->addWidget(runMutexBtn);
    buttonLayout->addWidget(runSemBtn);

    layout->addLayout(buttonLayout);

    // Junta todo arriba
    layout->addSpacing(5);

    // Aquí el stretch: el espacio sobrante va abajo
    layout->addStretch();

    // Tabla de resultados
    syncTable = new QTableWidget();
    syncTable->setColumnCount(5);
    syncTable->setHorizontalHeaderLabels({"Proceso", "Estado", "Recurso", "Acción", "Ciclo"});
    syncTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #ffffff;"
        "  border: 2px solid #c4dafa;"
        "  border-radius: 8px;"
        "  font-size: 12px;"
        "  selection-background-color: #e3f0fc;"
        "  selection-color: #263238;"
        "}"
        "QHeaderView::section {"
        "  background-color: #6fa8dc;"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "  border: none;"
        "  padding: 4px;"
        "}"
        "QTableWidget::item {"
        "  padding: 3px;"
        "}"
    );
    syncTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    syncTable->verticalHeader()->setVisible(false);
    syncTable->setShowGrid(false);
    syncTable->setAlternatingRowColors(true);
    syncTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    syncTable->setSelectionMode(QAbstractItemView::SingleSelection);
    syncTable->setMinimumHeight(120);
    syncTable->setMaximumHeight(180);

    layout->addWidget(syncTable);
    simulationArea = new QWidget(this);
    simulationArea->setMinimumHeight(180);
    simulationArea->setStyleSheet("background: #fff; border-radius: 8px; border: 1px solid #e3f0fc;");
    QVBoxLayout* simLayout = new QVBoxLayout(simulationArea);
    simLayout->setSpacing(8);
    simLayout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(simulationArea);

    // Conexiones
    connect(loadProcBtn, &QPushButton::clicked, this, &SynchronizationSimulatorWidget::loadProcessesFromDialog);
    connect(loadResBtn, &QPushButton::clicked, this, &SynchronizationSimulatorWidget::loadResourcesFromDialog);
    connect(loadActBtn, &QPushButton::clicked, this, &SynchronizationSimulatorWidget::loadActionsFromDialog);
    connect(runMutexBtn, &QPushButton::clicked, [this]() { runSynchronization("Mutex Lock"); });
    connect(runSemBtn, &QPushButton::clicked, [this]() { runSynchronization("Semaphore"); });
}

void SynchronizationSimulatorWidget::loadProcessesFromDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Cargar Procesos", "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;
    processes = loadProcesses(fileName);
    QMessageBox::information(this, "Procesos cargados", QString("Se cargaron %1 procesos.").arg(processes.size()));
}

void SynchronizationSimulatorWidget::loadResourcesFromDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Cargar Recursos", "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;
    resources = loadResources(fileName);
    QMessageBox::information(this, "Recursos cargados", QString("Se cargaron %1 recursos.").arg(resources.size()));
}

void SynchronizationSimulatorWidget::loadActionsFromDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Cargar Acciones", "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;
    actions = loadActions(fileName);
    QMessageBox::information(this, "Acciones cargadas", QString("Se cargaron %1 acciones.").arg(actions.size()));
}

void SynchronizationSimulatorWidget::runSynchronization(const QString &mechanism)
{
    if (processes.empty() || resources.empty() || actions.empty())
    {
        QMessageBox::warning(this, "Advertencia", "Faltan procesos, recursos o acciones por cargar.");
        return;
    }
    delete syncMechanism;
    if (mechanism == "Mutex Lock")
        syncMechanism = new MutexLock(resources);
    else if (mechanism == "Semaphore")
        syncMechanism = new Semaphore(resources);

    auto events = SynchronizationSimulator::simulateSynchronization(processes, resources, actions, syncMechanism);
    syncTable->setRowCount(events.size());
    for (int i = 0; i < events.size(); ++i)
    {
        const auto &event = events[i];
        syncTable->setItem(i, 0, new QTableWidgetItem(event.pid));
        syncTable->setItem(i, 1, new QTableWidgetItem(event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING"));
        syncTable->setItem(i, 2, new QTableWidgetItem(event.resource));
        syncTable->setItem(i, 3, new QTableWidgetItem(event.action_type));
        syncTable->setItem(i, 4, new QTableWidgetItem(QString::number(event.cycle)));
    }
    showSimulationEvents(events);
}

void SynchronizationSimulatorWidget::showSimulationEvents(const std::vector<SyncEvent>& events) {
    // Limpia el área
    QLayout* oldLayout = simulationArea->layout();
    if (oldLayout) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    // Crear nuevo layout para la simulación
    QVBoxLayout* mainLayout = new QVBoxLayout(simulationArea);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    if (events.empty()) {
        mainLayout->addWidget(new QLabel("No hay eventos para mostrar"));
        return;
    }

    // Encontrar el número máximo de ciclos y procesos únicos
    int maxCycle = 0;
    QSet<QString> uniqueProcesses;

    for (const auto& event : events) {
        maxCycle = std::max(maxCycle, event.cycle);
        uniqueProcesses.insert(event.pid);
    }

    QStringList processList = uniqueProcesses.values();
    std::sort(processList.begin(), processList.end());

    // Crear estructura de datos para organizar eventos
    // [proceso][ciclo] = evento
    QMap<QString, QMap<int, SyncEvent>> processEvents;

    for (const auto& event : events) {
        // Solo mostrar eventos ACCESSED para la visualización principal
        if (event.state == ProcessState::ACCESSED) {
            processEvents[event.pid].insert(event.cycle, event);
        }
    }

    // Crear encabezado con números de ciclo
    QHBoxLayout* headerLayout = new QHBoxLayout();

    // Espacio para la columna de procesos
    QLabel* cornerLabel = new QLabel("");
    cornerLabel->setFixedWidth(60);
    cornerLabel->setStyleSheet("border: 1px solid #ddd; background: #f5f5f5; font-weight: bold;");
    headerLayout->addWidget(cornerLabel);

    // Números de ciclo como encabezados
    for (int cycle = 0; cycle <= maxCycle; cycle++) {
        QLabel* cycleLabel = new QLabel(QString::number(cycle));
        cycleLabel->setFixedWidth(80);
        cycleLabel->setFixedHeight(30);
        cycleLabel->setAlignment(Qt::AlignCenter);
        cycleLabel->setStyleSheet(
            "border: 1px solid #ddd; "
            "background: #6fa8dc; "
            "color: white; "
            "font-weight: bold; "
            "font-size: 12px;"
        );
        headerLayout->addWidget(cycleLabel);
    }
    headerLayout->addStretch();

    QWidget* headerWidget = new QWidget();
    headerWidget->setLayout(headerLayout);
    mainLayout->addWidget(headerWidget);

    // Crear filas para cada proceso
    for (const QString& processId : processList) {
        QHBoxLayout* rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(0);

        // Etiqueta del proceso
        QLabel* processLabel = new QLabel(processId);
        processLabel->setFixedWidth(60);
        processLabel->setFixedHeight(40);
        processLabel->setAlignment(Qt::AlignCenter);

        // Obtener color del proceso de los eventos
        QColor processColor = Qt::lightGray;
        for (const auto& event : events) {
            if (event.pid == processId) {
                processColor = event.color;
                break;
            }
        }

        processLabel->setStyleSheet(QString(
            "border: 1px solid #ddd; "
            "background: %1; "
            "font-weight: bold; "
            "font-size: 11px; "
            "color: #333;"
        ).arg(processColor.name()));

        rowLayout->addWidget(processLabel);

        // Celdas para cada ciclo
        for (int cycle = 0; cycle <= maxCycle; cycle++) {
            QLabel* cellLabel = new QLabel();
            cellLabel->setFixedWidth(80);
            cellLabel->setFixedHeight(40);
            cellLabel->setAlignment(Qt::AlignCenter);

            if (processEvents[processId].contains(cycle)) {
                const SyncEvent& event = processEvents[processId][cycle];

                // Texto del evento
                QString eventText = QString("%1\n%2").arg(event.action_type, event.resource);
                cellLabel->setText(eventText);

                // Color según el tipo de acción
                QString bgColor = (event.action_type == "READ") ? "#b6f7b6" : "#ffd580";

                cellLabel->setStyleSheet(QString(
                    "border: 1px solid #ddd; "
                    "background: %1; "
                    "font-weight: bold; "
                    "font-size: 10px; "
                    "color: #222; "
                    "border-radius: 4px;"
                ).arg(bgColor));
            } else {
                // Celda vacía
                cellLabel->setStyleSheet(
                    "border: 1px solid #ddd; "
                    "background: white;"
                );
            }

            rowLayout->addWidget(cellLabel);
        }

        rowLayout->addStretch();

        QWidget* rowWidget = new QWidget();
        rowWidget->setLayout(rowLayout);
        mainLayout->addWidget(rowWidget);
    }

    // Agregar información adicional sobre procesos en espera
    QLabel* waitingInfo = new QLabel();
    QString waitingText = "<b>Estados de espera:</b><br>";

    QMap<int, QStringList> waitingByCycle;
    for (const auto& event : events) {
        if (event.state == ProcessState::WAITING) {
            waitingByCycle[event.cycle].append(
                QString("%1 esperando %2 (%3)").arg(event.pid, event.resource, event.action_type)
            );
        }
    }

    if (!waitingByCycle.isEmpty()) {
        for (auto it = waitingByCycle.begin(); it != waitingByCycle.end(); ++it) {
            waitingText += QString("Ciclo %1: %2<br>").arg(it.key()).arg(it.value().join(", "));
        }
    } else {
        waitingText += "No hay procesos en espera.";
    }

    waitingInfo->setText(waitingText);
    waitingInfo->setStyleSheet(
        "background: #fff3cd; "
        "border: 1px solid #ffeaa7; "
        "border-radius: 4px; "
        "padding: 8px; "
        "font-size: 11px; "
        "margin-top: 10px;"
    );
    waitingInfo->setWordWrap(true);
    mainLayout->addWidget(waitingInfo);

    mainLayout->addStretch();
}