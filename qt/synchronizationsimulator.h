#pragma once
#include <QWidget>
#include <QTableWidget>
#include <vector>


class SynchronizationMechanism;

class SynchronizationSimulatorWidget : public QWidget {
    Q_OBJECT
public:
    explicit SynchronizationSimulatorWidget(QWidget *parent = nullptr);
    ~SynchronizationSimulatorWidget();

private:
    void setupUI();
    void loadProcessesFromDialog();
    void loadResourcesFromDialog();
    void loadActionsFromDialog();
    void runSynchronization(const QString &mechanism);

    QTableWidget *syncTable;
    std::vector<Resource> resources;
    std::vector<Action> actions;
    std::vector<Process> processes;
    SynchronizationMechanism *syncMechanism;
};