#include "GanttChartWidget.h"
#include <QPainter>
#include <QFont>
#include <QScrollArea>
#include <QSize>

GanttChartWidget::GanttChartWidget(QWidget* parent)
    : QWidget(parent), currentTime(0), maxTime(0), isAnimating(false), animationSpeed(500) {
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); // <-- Agrega esto
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &GanttChartWidget::updateAnimation);
    setStyleSheet("background-color: white; border: 2px solid #e9ecef; border-radius: 10px;");
}

void GanttChartWidget::updateSize() {
    if (timeline.empty()) return;

    int maxTime = 0;
    for (const auto& slice : timeline) {
        maxTime = std::max(maxTime, slice.start_time + slice.duration);
    }

    int minWidth = maxTime * 40 + 100;
    setMinimumWidth(minWidth); // Esto permite que el widget crezca
    setMinimumHeight(200);

    update();
}

void GanttChartWidget::setTimeline(const std::vector<ExecutionSlice>& newTimeline) {
    timeline = newTimeline;
    maxTime = 0;
    for (const auto& slice : timeline) {
        maxTime = std::max(maxTime, slice.start_time + slice.duration);
    }
    currentTime = 0;
    updateSize(); // <-- Llama aquí
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

    int margin = 40;
    int chartHeight = height() - 2 * margin;
    int chartWidth = width() - 2 * margin;
    int timeUnit = 40; // Fijo para que el scroll funcione bien

    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(margin, height() - margin, width() - margin, height() - margin);

    painter.setFont(QFont("Arial", 10));
    for (int t = 0; t <= maxTime; t++) {
        int x = margin + t * timeUnit;
        painter.drawLine(x, height() - margin - 5, x, height() - margin + 5);
        painter.drawText(x - 10, height() - margin + 20, QString::number(t));
    }

    if (isAnimating && currentTime <= maxTime) {
        int currentX = margin + currentTime * timeUnit;
        painter.setPen(QPen(Qt::red, 3));
        painter.drawLine(currentX, margin, currentX, height() - margin);
    }

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

QScrollArea* GanttChartWidget::createScrollArea() {
    QScrollArea* ganttScrollArea = new QScrollArea();
    ganttScrollArea->setWidget(this);
    ganttScrollArea->setWidgetResizable(false);  // Volver a false
    ganttScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  // Cambiar a AlwaysOn
    ganttScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ganttScrollArea->setFixedHeight(220);
    ganttScrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background-color: white;"
        "}"
        "QScrollBar:horizontal {"
        "    background: #f8f9fa;"
        "    height: 14px;"
        "    border-radius: 7px;"
        "    margin: 2px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: #6c757d;"
        "    border-radius: 7px;"
        "    min-width: 30px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "    background: #495057;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    border: none;"
        "    background: none;"
        "}"
    );
    return ganttScrollArea;
}

QSize GanttChartWidget::sizeHint() const {
    // Ajusta el tamaño sugerido según tus necesidades
    return QSize(800, 200);
}