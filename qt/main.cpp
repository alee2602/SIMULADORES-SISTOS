#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QFrame>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QTextEdit>
#include <QScrollArea>
#include <QPainter>
#include <QTimer>
#include <QProgressBar>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStackedWidget>
#include <QButtonGroup>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <map>
#include "synchronizer.h"

using namespace std;

// ==============================
// GANTT CHART WIDGET
// ==============================
class GanttChartWidget : public QWidget {
    Q_OBJECT

private:
    vector<ExecutionSlice> timeline;
    int currentTime;
    int maxTime;
    QTimer* animationTimer;
    bool isAnimating;
    int animationSpeed;

public:
    GanttChartWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(200);
        currentTime = 0;
        maxTime = 0;
        isAnimating = false;
        animationSpeed = 500;
        
        animationTimer = new QTimer(this);
        connect(animationTimer, &QTimer::timeout, this, &GanttChartWidget::updateAnimation);
        
        setStyleSheet("background-color: white; border: 2px solid #e9ecef; border-radius: 10px;");
    }

    void setTimeline(const vector<ExecutionSlice>& newTimeline) {
        timeline = newTimeline;
        maxTime = 0;
        for (const auto& slice : timeline) {
            maxTime = max(maxTime, slice.start_time + slice.duration);
        }
        currentTime = 0;
        update();
    }

    void startAnimation() {
        if (timeline.empty()) return;
        
        isAnimating = true;
        currentTime = 0;
        animationTimer->start(animationSpeed);
    }

    void stopAnimation() {
        isAnimating = false;
        animationTimer->stop();
        currentTime = maxTime;
        update();
    }

    void setAnimationSpeed(int speed) {
        animationSpeed = speed;
        if (animationTimer->isActive()) {
            animationTimer->setInterval(speed);
        }
    }

private slots:
    void updateAnimation() {
        if (currentTime >= maxTime) {
            stopAnimation();
            return;
        }
        currentTime++;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        if (timeline.empty()) {
            painter.setFont(QFont("Arial", 14));
            painter.drawText(rect(), Qt::AlignCenter, "No processes to display");
            return;
        }

        int margin = 40;
        int chartHeight = height() - 2 * margin;
        int chartWidth = width() - 2 * margin;
        int timeUnit = max(1, chartWidth / max(1, maxTime));

        // Draw time axis
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(margin, height() - margin, width() - margin, height() - margin);

        // Draw time labels
        painter.setFont(QFont("Arial", 10));
        for (int t = 0; t <= maxTime; t++) {
            int x = margin + t * timeUnit;
            painter.drawLine(x, height() - margin - 5, x, height() - margin + 5);
            painter.drawText(x - 10, height() - margin + 20, QString::number(t));
        }

        // Draw current time indicator
        if (isAnimating && currentTime <= maxTime) {
            int currentX = margin + currentTime * timeUnit;
            painter.setPen(QPen(Qt::red, 3));
            painter.drawLine(currentX, margin, currentX, height() - margin);
        }

        // Draw process blocks
        int blockHeight = chartHeight / 3;
        int yPos = margin + blockHeight / 2;

        for (const auto& slice : timeline) {
            if (!isAnimating || slice.start_time < currentTime) {
                int x = margin + slice.start_time * timeUnit;
                int width = slice.duration * timeUnit;
                
                if (isAnimating && slice.start_time + slice.duration > currentTime) {
                    width = (currentTime - slice.start_time) * timeUnit;
                }

                QRect rect(x, yPos, width, blockHeight);
                painter.fillRect(rect, slice.color);
                painter.setPen(QPen(Qt::black));
                painter.drawRect(rect);

                if (width > 20) {
                    painter.setPen(Qt::white);
                    painter.setFont(QFont("Arial", 12, QFont::Bold));
                    painter.drawText(rect, Qt::AlignCenter, slice.pid);
                }
            }
        }

        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 10));
        painter.drawText(10, 20, QString("Current Time: %1").arg(currentTime));
    }
};


class SchedulingAlgorithms {
public:
    // First In First Out (FIFO)
    static vector<ExecutionSlice> runFIFO(vector<Process>& processes) {
        sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
            return a.arrival_time < b.arrival_time;
        });

        vector<ExecutionSlice> timeline;
        int currentTime = 0;

        for (auto& p : processes) {
            if (currentTime < p.arrival_time) {
                currentTime = p.arrival_time;
            }
            p.start_time = currentTime;
            p.finish_time = currentTime + p.burst_time;
            p.waiting_time = p.start_time - p.arrival_time;
            
            timeline.push_back({p.pid, currentTime, p.burst_time, p.color});
            currentTime += p.burst_time;
        }

        return timeline;
    }

    // Shortest Job First (SJF)
    static vector<ExecutionSlice> runSJF(vector<Process>& processes) {
        vector<ExecutionSlice> timeline;
        vector<Process> remaining = processes;
        vector<Process> ready_queue;
        vector<Process> executed;
        int currentTime = 0;

        while (!remaining.empty() || !ready_queue.empty()) {
            for (auto it = remaining.begin(); it != remaining.end();) {
                if (it->arrival_time <= currentTime) {
                    ready_queue.push_back(*it);
                    it = remaining.erase(it);
                } else {
                    ++it;
                }
            }

            if (!ready_queue.empty()) {
                auto shortest = min_element(ready_queue.begin(), ready_queue.end(),
                    [](const Process& a, const Process& b) {
                        return a.burst_time < b.burst_time;
                    });

                Process current = *shortest;
                ready_queue.erase(shortest);

                current.start_time = currentTime;
                current.finish_time = currentTime + current.burst_time;
                current.waiting_time = current.start_time - current.arrival_time;

                timeline.push_back({current.pid, currentTime, current.burst_time, current.color});
                
                currentTime = current.finish_time;
                executed.push_back(current);
            } else {
                currentTime++;
            }
        }

        processes = executed;
        return timeline;
    }

    // Shortest Remaining Time First (SRTF)
    static vector<ExecutionSlice> runSRTF(vector<Process>& processes) {
        vector<ExecutionSlice> timeline;
        vector<Process> remaining = processes;
        vector<Process> ready_queue;
        map<QString, int> remaining_bt;
        map<QString, int> start_times;
        vector<Process> executed;
        int currentTime = 0;

        for (auto& p : processes) {
            remaining_bt[p.pid] = p.burst_time;
        }

        while (!ready_queue.empty() || !remaining.empty()) {
            // Add arrived processes
            for (auto it = remaining.begin(); it != remaining.end();) {
                if (it->arrival_time <= currentTime) {
                    ready_queue.push_back(*it);
                    it = remaining.erase(it);
                } else {
                    ++it;
                }
            }

            if (!ready_queue.empty()) {
                // Find process with shortest remaining time
                auto shortest = min_element(ready_queue.begin(), ready_queue.end(),
                    [&](const Process& a, const Process& b) {
                        return remaining_bt[a.pid] < remaining_bt[b.pid];
                    });

                Process current = *shortest;
                
                if (start_times.find(current.pid) == start_times.end()) {
                    start_times[current.pid] = currentTime;
                }

                // Execute for 1 time unit
                timeline.push_back({current.pid, currentTime, 1, current.color});
                currentTime++;
                remaining_bt[current.pid]--;

                if (remaining_bt[current.pid] == 0) {
                    ready_queue.erase(shortest);
                    current.finish_time = currentTime;
                    current.start_time = start_times[current.pid];
                    current.waiting_time = (current.finish_time - current.arrival_time) - current.burst_time;
                    executed.push_back(current);
                }
            } else {
                currentTime++;
            }
        }

        processes = executed;
        return timeline;
    }

    // Round Robin
    static vector<ExecutionSlice> runRoundRobin(vector<Process>& processes, int quantum) {
        vector<ExecutionSlice> timeline;
        queue<Process> ready_queue;
        vector<Process> remaining = processes;
        map<QString, int> remaining_bt;
        map<QString, int> start_times;
        vector<Process> executed;
        int currentTime = 0;

        for (auto& p : processes) {
            remaining_bt[p.pid] = p.burst_time;
        }

        while (!ready_queue.empty() || !remaining.empty()) {
            for (auto it = remaining.begin(); it != remaining.end();) {
                if (it->arrival_time <= currentTime) {
                    ready_queue.push(*it);
                    it = remaining.erase(it);
                } else {
                    ++it;
                }
            }

            if (!ready_queue.empty()) {
                Process current = ready_queue.front();
                ready_queue.pop();

                if (start_times.find(current.pid) == start_times.end()) {
                    start_times[current.pid] = currentTime;
                }

                int exec_time = min(quantum, remaining_bt[current.pid]);
                timeline.push_back({current.pid, currentTime, exec_time, current.color});

                currentTime += exec_time;
                remaining_bt[current.pid] -= exec_time;

                for (auto it = remaining.begin(); it != remaining.end();) {
                    if (it->arrival_time <= currentTime) {
                        ready_queue.push(*it);
                        it = remaining.erase(it);
                    } else {
                        ++it;
                    }
                }

                if (remaining_bt[current.pid] > 0) {
                    ready_queue.push(current);
                } else {
                    current.finish_time = currentTime;
                    current.waiting_time = (current.finish_time - current.arrival_time) - current.burst_time;
                    current.start_time = start_times[current.pid];
                    executed.push_back(current);
                }
            } else {
                currentTime++;
            }
        }

        processes = executed;
        return timeline;
    }

    // Priority Scheduling with Aging
    static vector<ExecutionSlice> runPriority(vector<Process>& processes, bool aging = true) {
        vector<ExecutionSlice> timeline;
        vector<Process> remaining = processes;
        vector<Process> ready_queue;
        vector<Process> executed;
        int currentTime = 0;
        int agingCounter = 0;

        while (!remaining.empty() || !ready_queue.empty()) {
            for (auto it = remaining.begin(); it != remaining.end();) {
                if (it->arrival_time <= currentTime) {
                    ready_queue.push_back(*it);
                    it = remaining.erase(it);
                } else {
                    ++it;
                }
            }

            if (!ready_queue.empty()) {
                // Apply aging every 5 time units
                if (aging && agingCounter % 5 == 0 && agingCounter > 0) {
                    for (auto& p : ready_queue) {
                        if (p.priority > 1) p.priority--; // Increase priority (lower number = higher priority)
                    }
                }

                // Find highest priority process (lowest number)
                auto highest = min_element(ready_queue.begin(), ready_queue.end(),
                    [](const Process& a, const Process& b) {
                        return a.priority < b.priority;
                    });

                Process current = *highest;
                ready_queue.erase(highest);

                current.start_time = currentTime;
                current.finish_time = currentTime + current.burst_time;
                current.waiting_time = current.start_time - current.arrival_time;

                timeline.push_back({current.pid, currentTime, current.burst_time, current.color});
                
                currentTime = current.finish_time;
                executed.push_back(current);
            } else {
                currentTime++;
            }
            agingCounter++;
        }

        processes = executed;
        return timeline;
    }
};

// ==============================
// MAIN APPLICATION CLASS
// ==============================
class ProcessSimulator : public QWidget {
    Q_OBJECT

private:
    vector<Process> processes;
    vector<Resource> resources;
    vector<Action> actions;
    
    // UI Components
    QStackedWidget* mainStack;
    QWidget* menuWidget;
    QWidget* schedulingWidget;
    QWidget* synchronizationWidget;
    
    GanttChartWidget* ganttChart;
    QTableWidget* processTable;
    QTableWidget* metricsTable;
    QTableWidget* syncTable;
    QLabel* statusLabel;

    SynchronizationMechanism* syncMechanism;

public:
    ProcessSimulator(QWidget* parent = nullptr) : QWidget(parent), syncMechanism(nullptr) {
        setupUI();
        generateSampleProcesses();
        generateSampleResources();
        generateSampleActions();
    }

    ~ProcessSimulator() {
        delete syncMechanism;
    }

private:
    void setupUI() {
        setWindowTitle("Advanced Process Scheduling & Synchronization Simulator");
        setFixedSize(1400, 1000);
        setStyleSheet("background-color: #f8f9fa;");

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainStack = new QStackedWidget();
        
        setupMenuWidget();
        setupSchedulingWidget();
        setupSynchronizationWidget();
        
        mainLayout->addWidget(mainStack);
    }

    // Funciones auxiliares para manipular colores pastel
    QString lightenColor(const QString& color) {
        QColor c(color);
        return c.lighter(120).name();
    }
    QString darkenColor(const QString& color) {
        QColor c(color);
        return c.darker(120).name();
    }

    // Botón de menú sin HTML, usando layout y widgets
    QPushButton* createMenuButton(const QString& text, const QString& color, const QString& description) {
        QPushButton* btn = new QPushButton();
        btn->setMinimumSize(900, 140);
        btn->setMaximumSize(1200, 140);

        QWidget* content = new QWidget();
        content->setAttribute(Qt::WA_TranslucentBackground); 
        QVBoxLayout* layout = new QVBoxLayout(content);
        layout->setContentsMargins(20, 15, 20, 15);
        layout->setSpacing(5);

        QLabel* titleLabel = new QLabel(text);
        titleLabel->setFont(QFont("Arial", 22, QFont::Bold));
        titleLabel->setStyleSheet("color: #222; background: transparent;"); // Texto negro
        titleLabel->setAlignment(Qt::AlignCenter);

        QLabel* descLabel = new QLabel(description);
        descLabel->setFont(QFont("Arial", 14));
        descLabel->setStyleSheet("color: #222; background: transparent;"); // Texto negro
        descLabel->setAlignment(Qt::AlignCenter);
        descLabel->setWordWrap(true);

        layout->addWidget(titleLabel);
        layout->addWidget(descLabel);

        btn->setStyleSheet(QString(
            "QPushButton { "
            "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, "
            "stop: 0 %1, stop: 1 %2); "
            "border: none; border-radius: 18px; "
            "padding: 0px; margin: 10px; "
            "color: #222; " 
            "}"
            "QPushButton:hover { "
            "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, "
            "stop: 0 %2, stop: 1 %1); "
            "color: #111;"
            "}"
            "QPushButton:pressed { "
            "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, "
            "stop: 0 %3, stop: 1 %4); "
            "color: #000;"
            "}"
        ).arg(color)
         .arg(lightenColor(color))
         .arg(darkenColor(color))
         .arg(color));

        QVBoxLayout* btnLayout = new QVBoxLayout(btn);
        btnLayout->addWidget(content);
        btnLayout->setContentsMargins(0, 0, 0, 0);

        return btn;
    }

QPushButton* createButton(const QString& text, const QString& color) {
    QPushButton* btn = new QPushButton(text);
    btn->setMinimumSize(140, 45);

    btn->setStyleSheet(QString(
        "QPushButton { "
        "background-color: %1; "
        "color: #000; "  // Texto negro
        "border: 2px solid %1; "
        "border-radius: 8px; font-weight: bold; "
        "padding: 12px; font-size: 14px; }"
        "QPushButton:hover { "
        "background-color: %2; "
        "color: #000; }"
        "QPushButton:pressed { "
        "background-color: %3; }"
    ).arg(color)
     .arg(QColor(color).lighter(110).name())
     .arg(QColor(color).darker(110).name()));

    return btn;
}
    void setupMenuWidget() {
        menuWidget = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(menuWidget);
        layout->setSpacing(20);
        layout->setContentsMargins(50, 30, 50, 30);

        QLabel* title = new QLabel("PROCESS SIMULATOR");
        title->setAlignment(Qt::AlignCenter);
        title->setFont(QFont("Arial", 32, QFont::Bold));
        title->setStyleSheet(
            "color: black; "
            "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, "
            "stop: 0 rgb(168, 220, 230), stop: 1 rgb(184, 231, 255)); "
            "border-radius: 20px; padding: 30px; margin: 20px;"
        );

        QLabel* subtitle = new QLabel("Choose Simulation Type");
        subtitle->setAlignment(Qt::AlignCenter);
        subtitle->setFont(QFont("Arial", 18));
        subtitle->setStyleSheet("color: #6c757d; margin: 10px;");

        QVBoxLayout* buttonLayout = new QVBoxLayout();
        buttonLayout->setSpacing(25);
        buttonLayout->setAlignment(Qt::AlignCenter);

        QPushButton* schedulingBtn = createMenuButton(
            "Simulador de Algoritmos de Calendarización",
            "#c4dafa",
            "Simulate process scheduling algorithms like FIFO, SJF, Round Robin, and Priority"
        );

        QPushButton* syncBtn = createMenuButton(
            "Simulador de Mecanismos de Sincronización",
            "#c4e5fb",
            "Simulate synchronization mechanisms like Mutex Locks and Semaphores"
        );

        buttonLayout->addWidget(schedulingBtn);
        buttonLayout->addWidget(syncBtn);

        layout->addWidget(title);
        layout->addWidget(subtitle);
        layout->addStretch(1);
        layout->addLayout(buttonLayout);
        layout->addStretch(2);

        connect(schedulingBtn, &QPushButton::clicked, [this]() {
            mainStack->setCurrentWidget(schedulingWidget);
        });

        connect(syncBtn, &QPushButton::clicked, [this]() {
            mainStack->setCurrentWidget(synchronizationWidget);
        });

        mainStack->addWidget(menuWidget);
    }

    void setupSchedulingWidget() {
        schedulingWidget = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(schedulingWidget);

        // Header with back button
        QHBoxLayout* headerLayout = new QHBoxLayout();
        QPushButton* backBtn = createButton("← Back to Menu", "#6c757d");
        QLabel* title = new QLabel("PROCESS SCHEDULING ALGORITHMS");
        title->setFont(QFont("Arial", 20, QFont::Bold));
        title->setStyleSheet("color: #2c3e50;");
        
        headerLayout->addWidget(backBtn);
        headerLayout->addWidget(title, 1, Qt::AlignCenter);
        headerLayout->addStretch();

        // Algorithm Buttons
        QHBoxLayout* algoLayout = new QHBoxLayout();
        QPushButton* fifoBtn = createButton("First In First Out", "#FF6B6B");
        QPushButton* sjfBtn = createButton("Shortest Job First", "#4ECDC4");
        QPushButton* srtfBtn = createButton("Shortest Remaining Time", "#45B7D1");
        QPushButton* rrBtn = createButton("Round Robin", "#96CEB4");
        QPushButton* priorityBtn = createButton("Priority (with Aging)", "#FFEAA7");

        algoLayout->addWidget(fifoBtn);
        algoLayout->addWidget(sjfBtn);
        algoLayout->addWidget(srtfBtn);
        algoLayout->addWidget(rrBtn);
        algoLayout->addWidget(priorityBtn);

        // Process Management
        QHBoxLayout* processLayout = new QHBoxLayout();
        QPushButton* loadBtn = createButton("Load Processes", "#6c85bd");
        QPushButton* generateBtn = createButton("Generate Sample", "#70a1a8");
        
        processLayout->addWidget(loadBtn);
        processLayout->addWidget(generateBtn);
        processLayout->addStretch();

        // Gantt Chart
        ganttChart = new GanttChartWidget();
        
        // Animation Controls
        QHBoxLayout* animLayout = new QHBoxLayout();
        QPushButton* startBtn = createButton("▶ Start Animation", "#30c752");
        QPushButton* stopBtn = createButton("Stop Animation", "#dc3545");
        QSpinBox* speedSpinBox = new QSpinBox();
        speedSpinBox->setRange(100, 2000);
        speedSpinBox->setValue(500);
        speedSpinBox->setSuffix(" ms");
        
        animLayout->addWidget(new QLabel("Speed:"));
        animLayout->addWidget(speedSpinBox);
        animLayout->addWidget(startBtn);
        animLayout->addWidget(stopBtn);
        animLayout->addStretch();

        // Tables
        QHBoxLayout* tablesLayout = new QHBoxLayout();
        
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

        // Status
        statusLabel = new QLabel("Ready to simulate scheduling algorithms");
        statusLabel->setStyleSheet("color: #6c757d; padding: 10px; font-size: 14px;");

        // Add to layout
        layout->addLayout(headerLayout);
        layout->addLayout(algoLayout);
        layout->addLayout(processLayout);
        layout->addWidget(ganttChart);
        layout->addLayout(animLayout);
        layout->addLayout(tablesLayout);
        layout->addWidget(statusLabel);

        // Connect signals
        connect(backBtn, &QPushButton::clicked, [this]() {
            mainStack->setCurrentWidget(menuWidget);
        });
        connect(loadBtn, &QPushButton::clicked, this, &ProcessSimulator::loadProcesses);
        connect(generateBtn, &QPushButton::clicked, this, &ProcessSimulator::generateSampleProcesses);
        connect(fifoBtn, &QPushButton::clicked, this, &ProcessSimulator::runFIFO);
        connect(sjfBtn, &QPushButton::clicked, this, &ProcessSimulator::runSJF);
        connect(srtfBtn, &QPushButton::clicked, this, &ProcessSimulator::runSRTF);
        connect(rrBtn, &QPushButton::clicked, this, &ProcessSimulator::runRoundRobin);
        connect(priorityBtn, &QPushButton::clicked, this, &ProcessSimulator::runPriority);
        connect(startBtn, &QPushButton::clicked, ganttChart, &GanttChartWidget::startAnimation);
        connect(stopBtn, &QPushButton::clicked, ganttChart, &GanttChartWidget::stopAnimation);
        connect(speedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                ganttChart, &GanttChartWidget::setAnimationSpeed);

        mainStack->addWidget(schedulingWidget);
    }

    void setupSynchronizationWidget() {
        synchronizationWidget = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(synchronizationWidget);

        // Header with back button
        QHBoxLayout* headerLayout = new QHBoxLayout();
        QPushButton* backBtn = createButton("← Back to Menu", "#6c757d");
        QLabel* title = new QLabel("SYNCHRONIZATION MECHANISMS");
        title->setFont(QFont("Arial", 20, QFont::Bold));
        title->setStyleSheet("color: #2c3e50;");
        
        headerLayout->addWidget(backBtn);
        headerLayout->addWidget(title, 1, Qt::AlignCenter);
        headerLayout->addStretch();

        // Synchronization Buttons
        QHBoxLayout* syncLayout = new QHBoxLayout();
        syncLayout->addStretch(); 

        QPushButton* mutexBtn = createButton("Mutex Locks", "#e74c3c");
        QPushButton* semaphoreBtn = createButton("Semaphores", "#f39c12");
        mutexBtn->setMinimumSize(300, 60);
        semaphoreBtn->setMinimumSize(300, 60);

        syncLayout->addWidget(mutexBtn);
        syncLayout->addWidget(semaphoreBtn);

        syncLayout->addStretch(); 

        // Resource and Action Management
        QHBoxLayout* resourceLayout = new QHBoxLayout();
        QPushButton* loadResourcesBtn = createButton("Load Resources", "#3498db");
        QPushButton* generateResourcesBtn = createButton("Generate Sample Resources", "#2ecc71");
        
        resourceLayout->addWidget(loadResourcesBtn);
        resourceLayout->addWidget(generateResourcesBtn);
        resourceLayout->addStretch();

        // Synchronization Table
        syncTable = new QTableWidget();
        syncTable->setColumnCount(5);
        syncTable->setHorizontalHeaderLabels({"PID", "State", "Resource", "Action", "Cycle"});
        syncTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        syncTable->setStyleSheet("background-color: white; border: 1px solid #ddd; font-size: 14px;");

        // Results Area
        QTextEdit* resultsArea = new QTextEdit();
        resultsArea->setMaximumHeight(150);
        resultsArea->setStyleSheet("background-color: white; border: 1px solid #ddd;");
        resultsArea->setPlaceholderText("Synchronization results will appear here...");

        // Add to layout
        layout->addLayout(headerLayout);
        layout->addLayout(syncLayout);
        layout->addLayout(resourceLayout);
        layout->addWidget(new QLabel("Synchronization Events:"));
        layout->addWidget(syncTable);
        layout->addWidget(new QLabel("Results:"));
        layout->addWidget(resultsArea);

        // Connect signals
        connect(backBtn, &QPushButton::clicked, [this]() {
            mainStack->setCurrentWidget(menuWidget);
        });
        connect(mutexBtn, &QPushButton::clicked, [this]() {
            runSynchronization("Mutex Lock");
        });
        connect(semaphoreBtn, &QPushButton::clicked, [this]() {
            runSynchronization("Semaphore");
        });
        connect(generateResourcesBtn, &QPushButton::clicked, [this]() {
            generateSampleResources();
            generateSampleActions();
            QMessageBox::information(this, "Success", "Sample resources and actions generated!");
        });

        mainStack->addWidget(synchronizationWidget);
    }


    void generateSampleProcesses() {
        processes.clear();
        QStringList colors = {"#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4", "#FFEAA7", "#DDA0DD", "#98FB98"};
        
        processes = {
            Process("P1", 8, 0, 3, -1, -1, -1, -1, QColor(colors[0])),
            Process("P2", 4, 1, 1, -1, -1, -1, -1, QColor(colors[1])),
            Process("P3", 9, 2, 4, -1, -1, -1, -1, QColor(colors[2])),
            Process("P4", 5, 3, 2, -1, -1, -1, -1, QColor(colors[3])),
            Process("P5", 2, 4, 5, -1, -1, -1, -1, QColor(colors[4]))
        };

        updateProcessTable();
        statusLabel->setText("Sample processes generated successfully!");
    }

    void generateSampleResources() {
        resources = {
            {"Resource1", 2, 2},
            {"Resource2", 1, 1},
            {"Resource3", 3, 3}
        };
    }

    void generateSampleActions() {
        actions = {
            {"P1", "READ", "Resource1", 0},
            {"P2", "WRITE", "Resource1", 1},
            {"P3", "READ", "Resource2", 2},
            {"P4", "WRITE", "Resource2", 3},
            {"P5", "READ", "Resource3", 4}
        };
    }

    void updateProcessTable() {
        processTable->setRowCount(processes.size());
        for (int i = 0; i < processes.size(); ++i) {
            processTable->setItem(i, 0, new QTableWidgetItem(processes[i].pid));
            processTable->setItem(i, 1, new QTableWidgetItem(QString::number(processes[i].burst_time)));
            processTable->setItem(i, 2, new QTableWidgetItem(QString::number(processes[i].arrival_time)));
            processTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].priority)));
            
            QTableWidgetItem* colorItem = new QTableWidgetItem("");
            colorItem->setBackground(processes[i].color);
            processTable->setItem(i, 4, colorItem);
        }
    }

    void updateMetricsTable() {
        metricsTable->setRowCount(processes.size());
        double totalWaiting = 0;
        double totalTurnaround = 0;
        
        for (int i = 0; i < processes.size(); ++i) {
            metricsTable->setItem(i, 0, new QTableWidgetItem(processes[i].pid));
            metricsTable->setItem(i, 1, new QTableWidgetItem(QString::number(processes[i].start_time)));
            metricsTable->setItem(i, 2, new QTableWidgetItem(QString::number(processes[i].finish_time)));
            metricsTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].waiting_time)));
            
            totalWaiting += processes[i].waiting_time;
            totalTurnaround += (processes[i].finish_time - processes[i].arrival_time);
        }
        
        double avgWaiting = totalWaiting / processes.size();
        double avgTurnaround = totalTurnaround / processes.size();
        
        statusLabel->setText(QString("Average Waiting Time: %1 | Average Turnaround Time: %2")
                           .arg(avgWaiting, 0, 'f', 2)
                           .arg(avgTurnaround, 0, 'f', 2));
    }

    void loadProcesses() {
        QString fileName = QFileDialog::getOpenFileName(this, "Load Processes", "", "Text Files (*.txt)");
        if (fileName.isEmpty()) return;

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Could not open file!");
            return;
        }

        processes.clear();
        QTextStream in(&file);
        QStringList colors = {"#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4", "#FFEAA7", "#DDA0DD", "#98FB98"};
        int colorIndex = 0;

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(',');
            if (parts.size() >= 4) {
                QString pid = parts[0].trimmed();
                int burst = parts[1].toInt();
                int arrival = parts[2].toInt();
                int priority = parts[3].toInt();
                
                processes.push_back(Process(pid, burst, arrival, priority, -1, -1, -1, -1, 
                                          QColor(colors[colorIndex % colors.size()])));
                colorIndex++;
            }
        }

        updateProcessTable();
        statusLabel->setText(QString("Loaded %1 processes from file").arg(processes.size()));
    }

    void runSynchronization(const QString& mechanism) {
        if (processes.empty() || resources.empty() || actions.empty()) {
            QMessageBox::warning(this, "Warning", "No processes, resources, or actions loaded!");
            return;
        }

        delete syncMechanism;
        if (mechanism == "Mutex Lock") {
            syncMechanism = new MutexLock(resources);
        } else if (mechanism == "Semaphore") {
            syncMechanism = new Semaphore(resources);
        }

        vector<SyncEvent> events = SynchronizationSimulator::simulateSynchronization(
            processes, resources, actions, syncMechanism);

        // Update synchronization table
        syncTable->setRowCount(events.size());
        QString resultText = QString("=== %1 Simulation Results ===\n").arg(mechanism);
        
        for (int i = 0; i < events.size(); ++i) {
            const auto& event = events[i];
            syncTable->setItem(i, 0, new QTableWidgetItem(event.pid));
            syncTable->setItem(i, 1, new QTableWidgetItem(
                event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING"));
            syncTable->setItem(i, 2, new QTableWidgetItem(event.resource));
            syncTable->setItem(i, 3, new QTableWidgetItem(event.action_type));
            syncTable->setItem(i, 4, new QTableWidgetItem(QString::number(event.cycle)));
            
            resultText += QString("Cycle %1: Process %2 %3 %4 (%5)\n")
                         .arg(event.cycle)
                         .arg(event.pid)
                         .arg(event.action_type)
                         .arg(event.resource)
                         .arg(event.state == ProcessState::ACCESSED ? "ACCESSED" : "WAITING");
        }

        // Show results in a message box
        QMessageBox msgBox;
        msgBox.setWindowTitle("Synchronization Results");
        msgBox.setText(QString("%1 simulation completed successfully!").arg(mechanism));
        msgBox.setDetailedText(resultText);
        msgBox.exec();
    }

    // Scheduling Algorithm Slots
    void runFIFO() {
        if (processes.empty()) {
            QMessageBox::warning(this, "Warning", "No processes loaded!");
            return;
        }
        
        auto processesCopy = processes;
        auto timeline = SchedulingAlgorithms::runFIFO(processesCopy);
        processes = processesCopy;
        ganttChart->setTimeline(timeline);
        updateMetricsTable();
        statusLabel->setText("FIFO (First In First Out) algorithm executed successfully!");
    }

    void runSJF() {
        if (processes.empty()) {
            QMessageBox::warning(this, "Warning", "No processes loaded!");
            return;
        }
        
        auto processesCopy = processes;
        auto timeline = SchedulingAlgorithms::runSJF(processesCopy);
        processes = processesCopy;
        ganttChart->setTimeline(timeline);
        updateMetricsTable();
        statusLabel->setText("SJF (Shortest Job First) algorithm executed successfully!");
    }

    void runSRTF() {
        if (processes.empty()) {
            QMessageBox::warning(this, "Warning", "No processes loaded!");
            return;
        }
        
        auto processesCopy = processes;
        auto timeline = SchedulingAlgorithms::runSRTF(processesCopy);
        processes = processesCopy;
        ganttChart->setTimeline(timeline);
        updateMetricsTable();
        statusLabel->setText("SRTF (Shortest Remaining Time First) algorithm executed successfully!");
    }

    void runRoundRobin() {
        if (processes.empty()) {
            QMessageBox::warning(this, "Warning", "No processes loaded!");
            return;
        }
        
        bool ok;
        int quantum = QInputDialog::getInt(this, "Round Robin Quantum", 
                                         "Enter time quantum:", 2, 1, 100, 1, &ok);
        if (!ok) return;

        auto processesCopy = processes;
        auto timeline = SchedulingAlgorithms::runRoundRobin(processesCopy, quantum);
        processes = processesCopy;
        ganttChart->setTimeline(timeline);
        updateMetricsTable();
        statusLabel->setText(QString("Round Robin algorithm executed with quantum = %1!").arg(quantum));
    }

    void runPriority() {
        if (processes.empty()) {
            QMessageBox::warning(this, "Warning", "No processes loaded!");
            return;
        }
        
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Priority Scheduling",
                                                                "Enable aging mechanism?",
                                                                QMessageBox::Yes | QMessageBox::No);
        bool aging = (reply == QMessageBox::Yes);

        auto processesCopy = processes;
        auto timeline = SchedulingAlgorithms::runPriority(processesCopy, aging);
        processes = processesCopy;
        ganttChart->setTimeline(timeline);
        updateMetricsTable();
        statusLabel->setText(QString("Priority scheduling executed %1 aging!")
                           .arg(aging ? "with" : "without"));
    }
};

// ==============================
// MAIN FUNCTION
// ==============================
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application style
    app.setStyleSheet(
        "QWidget { font-family: 'Segoe UI', Arial, sans-serif; }"
        "QTableWidget { gridline-color: #e0e0e0; }"
        "QTableWidget::item { padding: 8px; }"
        "QTableWidget::item:selected { background-color: #3498db; color: white; }"
        "QHeaderView::section { background-color: #34495e; color: white; "
        "padding: 10px; border: none; font-weight: bold; }"
        "QSpinBox { padding: 5px; border: 1px solid #ddd; border-radius: 4px; }"
        "QLabel { color: #2c3e50; }"
    );

    ProcessSimulator simulator;
    simulator.show();

    return app.exec();
}

#include "main.moc"