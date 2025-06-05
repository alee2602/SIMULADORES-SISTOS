#include "GanttChartWidget.h"
#include <QPainter>
#include <QFont>
#include <QScrollArea>
#include <QSize>

GanttChartWidget::GanttChartWidget(QWidget* parent)
    : QWidget(parent), currentTime(0), maxTime(0), isAnimating(false), animationSpeed(500),
      avgWaitingTime(0), avgTurnaroundTime(0), showMetrics(false) {
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); // <-- Agrega esto
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &GanttChartWidget::updateAnimation);
    setStyleSheet("background-color: white; border: 2px solid #e9ecef; border-radius: 10px;");
}

void GanttChartWidget::setAlgorithmTitle(const QString& title) {
    algorithmTitle = title;
    update();  
}


void GanttChartWidget::updateSize() {
    if (timeline.empty()) return;

    int maxTime = 0;
    int maxLane = 0;
    for (const auto& slice : timeline) {
        maxTime = std::max(maxTime, slice.start_time + slice.duration);
        maxLane = std::max(maxLane, slice.lane);
    }

    int minWidth = maxTime * 40 + 100;
    int minHeight = 80 + (maxLane + 1) * 60; 
    setMinimumWidth(minWidth);
    setMinimumHeight(minHeight);

    update();
}

void GanttChartWidget::setTimeline(const std::vector<ExecutionSlice>& newTimeline) {
    timeline = newTimeline;
    maxTime = 0;
    for (const auto& slice : timeline) {
        maxTime = std::max(maxTime, slice.start_time + slice.duration);
    }
    currentTime = 0;
    updateSize(); 
    update();
}

void GanttChartWidget::startAnimation() {
    if (timeline.empty()) return;
    isAnimating = true;
    currentTime = 0;
    animationTimer->start(animationSpeed);
}

void GanttChartWidget::stopAnimation() {
    isAnimating = false;
    animationTimer->stop();
    currentTime = maxTime;
    update();
}

void GanttChartWidget::setAnimationSpeed(int speed) {
    animationSpeed = speed;
    if (animationTimer->isActive()) {
        animationTimer->setInterval(speed);
    }
}

void GanttChartWidget::updateAnimation() {
    if (currentTime >= maxTime) {
        stopAnimation();
        emit animationFinished();
        return;
    }
    currentTime++;
    update();
}

void GanttChartWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (timeline.empty()) {
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "No processes to display");
        return;
    }

    if (!algorithmTitle.isEmpty()) {
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.setPen(QColor("#2c3e50"));
    QRect titleRect(0, 10, width(), 30);
    painter.drawText(titleRect, Qt::AlignCenter, algorithmTitle);
    }

    // painter.setFont(QFont("Arial", 16, QFont::Bold));
    // painter.setPen(QColor("#2c3e50"));
    // QRect titleRect(0, 0, width(), 35);
    // painter.drawText(titleRect, Qt::AlignCenter, "Gantt Chart"); // <-- Elimina o comenta esta línea

    // Ajusta el margen superior para dejar espacio al título
    int margin = 60; // Antes era 40
    int chartHeight = height() - margin - 40; // 40 margen inferior
    int chartWidth = width() - 2 * 40;
    int timeUnit = (maxTime > 0) ? std::max(40, std::min(80, chartWidth / maxTime)) : 40;

    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(40, height() - 40, width() - 40, height() - 40);

    painter.setFont(QFont("Arial", 10));
    for (int t = 0; t <= maxTime; t++) {
        int x = 40 + t * timeUnit;
        painter.drawLine(x, height() - 45, x, height() - 35);
        painter.drawText(x - 10, height() - 20, QString::number(t));
    }

    if (isAnimating && currentTime <= maxTime) {
        int currentX = 40 + currentTime * timeUnit;
        painter.setPen(QPen(Qt::red, 3));
        painter.drawLine(currentX, margin, currentX, height() - 40);
    }

    int blockHeight = chartHeight / 3;
    int yPos = margin + blockHeight / 2;

    for (const auto& slice : timeline) {
        if (!isAnimating || slice.start_time < currentTime) {
            int x = 40 + slice.start_time * timeUnit;
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
    painter.drawText(10, margin - 10, QString("Current Time: %1").arg(currentTime));

    // --- Métricas abajo ---
    if (showMetrics && !timeline.empty()) {
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.setPen(QColor("#28a745"));
        QString metricsText = QString("Avg Waiting Time: %1 | Avg Completion Time: %2")
                             .arg(avgWaitingTime, 0, 'f', 2)
                             .arg(avgTurnaroundTime, 0, 'f', 2);
        QRect metricsRect = rect().adjusted(10, rect().height() - 30, -10, -5);
        painter.drawText(metricsRect, Qt::AlignCenter, metricsText);
    }
}

QScrollArea* GanttChartWidget::createScrollArea() {
    QScrollArea* ganttScrollArea = new QScrollArea();
    ganttScrollArea->setWidget(this);
    ganttScrollArea->setWidgetResizable(false);
    ganttScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ganttScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Cambia a AlwaysOn
    ganttScrollArea->setMinimumHeight(220);
    ganttScrollArea->setMinimumWidth(800);
    ganttScrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background-color: white;"
        "}"
        "QScrollBar:horizontal, QScrollBar:vertical {"
        "    background: #f8f9fa;"
        "    border-radius: 7px;"
        "    margin: 2px;"
        "}"
        "QScrollBar::handle:horizontal, QScrollBar::handle:vertical {"
        "    background: #6c757d;"
        "    border-radius: 7px;"
        "    min-width: 30px;"
        "    min-height: 30px;"
        "}"
        "QScrollBar::handle:horizontal:hover, QScrollBar::handle:vertical:hover {"
        "    background: #495057;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    border: none;"
        "    background: none;"
        "}"
    );
    return ganttScrollArea;
}

QSize GanttChartWidget::sizeHint() const {
    return QSize(800, 200);
}

bool GanttChartWidget::hasTimeline() const {
    return !timeline.empty();
}

void GanttChartWidget::setMetrics(double avgWaiting, double avgTurnaround) {
    avgWaitingTime = avgWaiting;
    avgTurnaroundTime = avgTurnaround;
    showMetrics = true;
    update(); // Redibujar el widget
}