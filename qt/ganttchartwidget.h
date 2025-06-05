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
    bool hasTimeline() const;
    void setAlgorithmTitle(const QString& title);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

public slots:
    void setMetrics(double avgWaiting, double avgTurnaround);

private slots:
    void updateAnimation();

signals:
    void animationFinished();

private:
    void updateSize();
    QString algorithmTitle;
    
    std::vector<ExecutionSlice> timeline;
    int currentTime;
    int maxTime;
    bool isAnimating;
    QTimer* animationTimer;
    int animationSpeed;
    bool comparisonMode = false;
    QStringList algorithmNames;
    double avgWaitingTime;
    double avgTurnaroundTime;
    bool showMetrics;

public:
    void setComparisonMode(bool enabled) { comparisonMode = enabled; }
    void setAlgorithmNames(const QStringList& names) { algorithmNames = names; }
    bool isComparisonMode() const { return comparisonMode; }
};

#endif