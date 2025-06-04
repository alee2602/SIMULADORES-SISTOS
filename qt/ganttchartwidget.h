#ifndef GANTTCHARTWIDGET_H
#define GANTTCHARTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QColor>
#include <vector>
#include "utils.h"

class GanttChartWidget : public QWidget {
    Q_OBJECT

public:
    GanttChartWidget(QWidget* parent = nullptr);
    void setTimeline(const std::vector<ExecutionSlice>& newTimeline);
    void startAnimation();
    void stopAnimation();
    void setAnimationSpeed(int speed);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateAnimation();

private:
    std::vector<ExecutionSlice> timeline;
    int currentTime;
    int maxTime;
    QTimer* animationTimer;
    bool isAnimating;
    int animationSpeed;
};

#endif