#ifndef GANTTCHARTWIDGET_H
#define GANTTCHARTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <vector>
#include <QColor>
#include <QString>
#include "utils.h"
#include <QScrollArea> 

class GanttChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit GanttChartWidget(QWidget* parent = nullptr);
    
    void setTimeline(const std::vector<ExecutionSlice>& timeline);
    void startAnimation();
    void stopAnimation();
    void setAnimationSpeed(int speed);
    QScrollArea* createScrollArea();

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private slots:
    void updateAnimation();

private:
    void updateSize();
    
    std::vector<ExecutionSlice> timeline;
    int currentTime;
    int maxTime;
    bool isAnimating;
    QTimer* animationTimer;
    int animationSpeed;
};

#endif