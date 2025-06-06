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
    }

    // Agrupa eventos por ciclo
    QMap<int, QList<const SyncEvent*>> eventsByCycle;
    for (const auto& ev : events) {
        eventsByCycle[ev.cycle].append(&ev);
    }

    QVBoxLayout* simLayout = qobject_cast<QVBoxLayout*>(simulationArea->layout());
    if (!simLayout) return;

    for (auto it = eventsByCycle.begin(); it != eventsByCycle.end(); ++it) {
        int cycle = it.key();
        const auto& evList = it.value();

        QHBoxLayout* row = new QHBoxLayout;
        QLabel* cycleLabel = new QLabel(QString::number(cycle));
        cycleLabel->setFixedWidth(24);
        cycleLabel->setAlignment(Qt::AlignCenter);
        row->addWidget(cycleLabel);

        for (const auto* ev : evList) {
            QLabel* proc = new QLabel(QString("%1<br>%2 %3").arg(ev->pid, ev->action_type, ev->resource));
            proc->setAlignment(Qt::AlignCenter);
            proc->setMinimumSize(70, 40);
            proc->setStyleSheet(QString(
                "background: %1; color: #222; border-radius: 8px; border: 1px solid #bbb; font-weight: bold;"
            ).arg(ev->action_type == "READ" ? "#b6f7b6" : "#ffd580"));
            row->addWidget(proc);
        }
        row->addStretch();
        QWidget* rowWidget = new QWidget;
        rowWidget->setLayout(row);
        simLayout->addWidget(rowWidget);
    }
    simLayout->addStretch();
}