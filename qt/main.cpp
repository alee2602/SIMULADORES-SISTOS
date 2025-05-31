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
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <map>

using namespace std;

// ==============================
// DATA STRUCTURES
// ==============================
struct Process {
    QString pid;
    int burst_time;
    int arrival_time;
    int priority;
    int start_time = -1;
    int finish_time = -1;
    int waiting_time = -1;
    int remaining_time = -1;
    QColor color;
};

struct ExecutionSlice {
    QString pid;
    int start_time;
    int duration;
    QColor color;
};

struct Resource {
    QString name;
    int count;
    int available;
};

struct Action {
    QString pid;
    QString type; // READ, WRITE
    QString resource;
    int cycle;
};

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
        animationSpeed = 500; // milliseconds
        
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
                
                // Only show completed portions during animation
                if (isAnimating && slice.start_time + slice.duration > currentTime) {
                    width = (currentTime - slice.start_time) * timeUnit;
                }

                QRect rect(x, yPos, width, blockHeight);
                painter.fillRect(rect, slice.color);
                painter.setPen(QPen(Qt::black));
                painter.drawRect(rect);

                // Draw process label
                if (width > 20) {
                    painter.setPen(Qt::white);
                    painter.setFont(QFont("Arial", 12, QFont::Bold));
                    painter.drawText(rect, Qt::AlignCenter, slice.pid);
                }
            }
        }

        // Draw legend
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 10));
        painter.drawText(10, 20, QString("Current Time: %1").arg(currentTime));
    }
};

// ==============================
// ALGORITHM IMPLEMENTATIONS
// ==============================
class SchedulingAlgorithms {
public:
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

    static vector<ExecutionSlice> runSJF(vector<Process>& processes) {
        vector<ExecutionSlice> timeline;
        vector<Process> remaining = processes;
        vector<Process> ready_queue;
        vector<Process> executed;
        int currentTime = 0;

        while (!remaining.empty() || !ready_queue.empty()) {
            // Add arrived processes to ready queue
            for (auto it = remaining.begin(); it != remaining.end();) {
                if (it->arrival_time <= currentTime) {
                    ready_queue.push_back(*it);
                    it = remaining.erase(it);
                } else {
                    ++it;
                }
            }

            if (!ready_queue.empty()) {
                // Find shortest job
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
            // Add arrived processes
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

                // Add new arrivals
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
    GanttChartWidget* ganttChart;
    QTableWidget* processTable;
    QTableWidget* metricsTable;
    QLabel* statusLabel;

public:
    ProcessSimulator(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        generateSampleProcesses(); // For testing
    }

private:
    void setupUI() {
        setWindowTitle("Advanced Process Scheduling Simulator");
        setFixedSize(1200, 900);
        setStyleSheet("background-color: white;");

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Title
        QLabel* title = new QLabel("PROCESS SCHEDULING SIMULATOR");
        title->setAlignment(Qt::AlignCenter);
        title->setFont(QFont("Arial", 24, QFont::Bold));
        title->setStyleSheet(
            "color: #2c3e50; background: #f8f9fa; border: 2px solid #e9ecef; "
            "border-radius: 15px; padding: 20px; margin: 10px;"
        );

        // Control Panel
        QHBoxLayout* controlLayout = new QHBoxLayout();
        
        QPushButton* loadBtn = createButton("Load Processes", "#6c85bd");
        QPushButton* fifoBtn = createButton("Run FIFO", "#70a1a8");
        QPushButton* sjfBtn = createButton("Run SJF", "#9ca3af");
        QPushButton* rrBtn = createButton("Run Round Robin", "#a8d5ba");

        controlLayout->addWidget(loadBtn);
        controlLayout->addWidget(fifoBtn);
        controlLayout->addWidget(sjfBtn);
        controlLayout->addWidget(rrBtn);

        // Gantt Chart
        ganttChart = new GanttChartWidget();
        
        // Animation Controls
        QHBoxLayout* animLayout = new QHBoxLayout();
        QPushButton* startBtn = createButton("Start Animation", "#28a745");
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

        // Tables Layout
        QHBoxLayout* tablesLayout = new QHBoxLayout();
        
        // Process Table
        processTable = new QTableWidget();
        processTable->setColumnCount(5);
        processTable->setHorizontalHeaderLabels({"PID", "Burst Time", "Arrival Time", "Priority", "Color"});
        processTable->horizontalHeader()->setStretchLastSection(true);
        
        // Metrics Table
        metricsTable = new QTableWidget();
        metricsTable->setColumnCount(4);
        metricsTable->setHorizontalHeaderLabels({"PID", "Start Time", "Finish Time", "Waiting Time"});
        metricsTable->horizontalHeader()->setStretchLastSection(true);

        tablesLayout->addWidget(processTable);
        tablesLayout->addWidget(metricsTable);

        // Status
        statusLabel = new QLabel("Ready to simulate processes");
        statusLabel->setStyleSheet("color: #6c757d; padding: 10px;");

        // Add all to main layout
        mainLayout->addWidget(title);
        mainLayout->addLayout(controlLayout);
        mainLayout->addWidget(ganttChart);
        mainLayout->addLayout(animLayout);
        mainLayout->addLayout(tablesLayout);
        mainLayout->addWidget(statusLabel);

        // Connect signals
        connect(loadBtn, &QPushButton::clicked, this, &ProcessSimulator::loadProcesses);
        connect(fifoBtn, &QPushButton::clicked, this, &ProcessSimulator::runFIFO);
        connect(sjfBtn, &QPushButton::clicked, this, &ProcessSimulator::runSJF);
        connect(rrBtn, &QPushButton::clicked, this, &ProcessSimulator::runRoundRobin);
        connect(startBtn, &QPushButton::clicked, ganttChart, &GanttChartWidget::startAnimation);
        connect(stopBtn, &QPushButton::clicked, ganttChart, &GanttChartWidget::stopAnimation);
        connect(speedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                ganttChart, &GanttChartWidget::setAnimationSpeed);
    }

    QPushButton* createButton(const QString& text, const QString& color) {
        QPushButton* btn = new QPushButton(text);
        btn->setMinimumSize(120, 40);
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; "
            "border-radius: 8px; font-weight: bold; padding: 10px; }"
            "QPushButton:hover { background-color: #555; }"
        ).arg(color));
        return btn;
    }

    void generateSampleProcesses() {
        processes.clear();
        QStringList colors = {"#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4", "#FFEAA7"};
        
        processes = {
            {"P1", 8, 0, 1, -1, -1, -1, -1, QColor(colors[0])},
            {"P2", 4, 1, 2, -1, -1, -1, -1, QColor(colors[1])},
            {"P3", 9, 2, 3, -1, -1, -1, -1, QColor(colors[2])},
            {"P4", 5, 3, 1, -1, -1, -1, -1, QColor(colors[3])},
            {"P5", 2, 4, 2, -1, -1, -1, -1, QColor(colors[4])}
        };
        
        updateProcessTable();
        statusLabel->setText("Sample processes loaded. Ready to simulate.");
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
        
        for (int i = 0; i < processes.size(); ++i) {
            metricsTable->setItem(i, 0, new QTableWidgetItem(processes[i].pid));
            metricsTable->setItem(i, 1, new QTableWidgetItem(QString::number(processes[i].start_time)));
            metricsTable->setItem(i, 2, new QTableWidgetItem(QString::number(processes[i].finish_time)));
            metricsTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].waiting_time)));
            totalWaiting += processes[i].waiting_time;
        }
        
        double avgWaiting = totalWaiting / processes.size();
        statusLabel->setText(QString("Average Waiting Time: %1").arg(avgWaiting, 0, 'f', 2));
    }

private slots:
    void loadProcesses() {
        QString filename = QFileDialog::getOpenFileName(this, "Load Processes", "", "Text Files (*.txt)");
        if (!filename.isEmpty()) {
            // Implementation for loading from file
            statusLabel->setText("File loading functionality - implement based on your format");
        }
    }

    void runFIFO() {
        if (processes.empty()) {
            QMessageBox::warning(this, "Warning", "No processes loaded!");
            return;
        }
        
        vector<Process> processCopy = processes;
        vector<ExecutionSlice> timeline = SchedulingAlgorithms::runFIFO(processCopy);
        processes = processCopy;
        
        ganttChart->setTimeline(timeline);
        updateMetricsTable();
        statusLabel->setText("FIFO Algorithm executed successfully");
    }

    void runSJF() {
        if (processes.empty()) {
            QMessageBox::warning(this, "Warning", "No processes loaded!");
            return;
        }
        
        vector<Process> processCopy = processes;
        vector<ExecutionSlice> timeline = SchedulingAlgorithms::runSJF(processCopy);
        processes = processCopy;
        
        ganttChart->setTimeline(timeline);
        updateMetricsTable();
        statusLabel->setText("SJF Algorithm executed successfully");
    }

    void runRoundRobin() {
        bool ok;
        int quantum = QInputDialog::getInt(this, "Round Robin Configuration", 
                                         "Enter quantum value:", 2, 1, 20, 1, &ok);
        if (!ok) return;
        
        if (processes.empty()) {
            QMessageBox::warning(this, "Warning", "No processes loaded!");
            return;
        }
        
        vector<Process> processCopy = processes;
        vector<ExecutionSlice> timeline = SchedulingAlgorithms::runRoundRobin(processCopy, quantum);
        processes = processCopy;
        
        ganttChart->setTimeline(timeline);
        updateMetricsTable();
        statusLabel->setText(QString("Round Robin (quantum=%1) executed successfully").arg(quantum));
    }
};

// ==============================
// MAIN FUNCTION
// ==============================
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ProcessSimulator simulator;
    simulator.show();

    return app.exec();
}

#include "main.moc"