#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include "utils.h"
#include <vector>
#include <map>
#include <queue>
#include <QTableWidget>
#include <QHeaderView>

enum class ProcessState {
    ACCESSED,
    WAITING,
    IDLE
};

struct SyncEvent {
    QString pid;
    QString resource;
    QString action_type; // READ, WRITE
    int cycle;
    ProcessState state;
    QColor color;
    
    SyncEvent(QString p, QString r, QString at, int c, ProcessState s, QColor col)
        : pid(p), resource(r), action_type(at), cycle(c), state(s), color(col) {}
};

struct ProcessSyncState {
    QString pid;
    ProcessState current_state;
    QString waiting_for_resource;
    int cycles_waiting;
    QColor color;

    // Constructor predeterminado
    ProcessSyncState() 
        : pid(""), current_state(ProcessState::IDLE), waiting_for_resource(""), cycles_waiting(0), color(Qt::white) {}

    // Constructor con parámetros
    ProcessSyncState(QString p, QColor c) 
        : pid(p), current_state(ProcessState::IDLE), waiting_for_resource(""), cycles_waiting(0), color(c) {}
};

class SynchronizationMechanism {
public:
    virtual ~SynchronizationMechanism() = default;
    virtual bool tryAcquire(const QString& resource, const QString& pid, const QString& action_type) = 0;
    virtual void release(const QString& resource, const QString& pid) = 0;
    virtual bool isAvailable(const QString& resource) const = 0;
    virtual void resetResources() = 0;
};

class MutexLock : public SynchronizationMechanism {
private:
    std::map<QString, QString> resource_owners; // resource -> process holding it
    std::vector<Resource> resources;

public:
    MutexLock(const std::vector<Resource>& res);
    bool tryAcquire(const QString& resource, const QString& pid, const QString& action_type) override;
    void release(const QString& resource, const QString& pid) override;
    bool isAvailable(const QString& resource) const override;
    void resetResources() override;
};

class Semaphore : public SynchronizationMechanism {
private:
    std::map<QString, int> resource_counts; // resource -> available count
    std::map<QString, std::vector<QString>> resource_holders; // resource -> list of processes holding it
    std::vector<Resource> resources;

public:
    Semaphore(const std::vector<Resource>& res);
    bool tryAcquire(const QString& resource, const QString& pid, const QString& action_type) override;
    void release(const QString& resource, const QString& pid) override;
    bool isAvailable(const QString& resource) const override;
    void resetResources() override;
    int getAvailableCount(const QString& resource) const;
};

class SynchronizationSimulator {
public:
    static std::vector<SyncEvent> simulateSynchronization(
        const std::vector<Process>& processes,
        const std::vector<Resource>& resources,
        const std::vector<Action>& actions,
        SynchronizationMechanism* mechanism
    );
    
    static std::vector<ProcessSyncState> getProcessStates(
        const std::vector<Process>& processes,
        const std::vector<SyncEvent>& events,
        int maxCycles
    );
};

class Synchronizer {
public:
    virtual ~Synchronizer() = default;
    // Métodos virtuales puros si es una interfaz
};

#endif
