#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include "utils.h"
#include <vector>
#include <map>
#include <queue>
#include <QTableWidget>
#include <QHeaderView>
#include <algorithm>

enum class ProcessState {
    ACCESSED,
    WAITING
};

struct SyncEvent {
    QString pid;
    QString resource;
    QString action_type; // READ, WRITE
    int cycle;
    ProcessState state;
    QColor color;

    SyncEvent() : cycle(0), state(ProcessState::WAITING), color(Qt::white) {}

    SyncEvent(QString p, QString r, QString at, int c, ProcessState s, QColor col)
        : pid(p), resource(r), action_type(at), cycle(c), state(s), color(col) {}
};

struct SyncProcessState {
    QString pid;
    ProcessState current_state;
    QString waiting_for_resource;
    int cycles_waiting;
    QColor color;

    SyncProcessState() 
        : pid(""), current_state(ProcessState::WAITING), waiting_for_resource(""), cycles_waiting(0), color(Qt::white) {}

    SyncProcessState(QString p, QColor c) 
        : pid(p), current_state(ProcessState::WAITING), waiting_for_resource(""), cycles_waiting(0), color(c) {}
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
    std::map<QString, QString> resource_owners;  // recurso -> PID (quien lo tiene)
    std::vector<Resource> resources;

public:
    MutexLock(const std::vector<Resource>& res);
    bool tryAcquire(const QString& resource, const QString& pid, const QString& action_type) override;
    void release(const QString& resource, const QString& pid) override;
    bool isAvailable(const QString& resource) const override;
    void resetResources() override;
    
    // Métodos auxiliares (mantengo para compatibilidad, pero simplificados)
    bool hasWriter(const QString& resource) const;
    bool hasReaders(const QString& resource) const;
};

class Semaphore : public SynchronizationMechanism {
private:
    std::map<QString, int> resource_counts;           // Cupos disponibles
    std::map<QString, std::vector<QString>> resource_holders; // Quién tiene el recurso
    std::map<QString, int> max_counts;                // Contador máximo original
    std::map<QString, QString> current_writers;       //  Escritor activo por recurso
    std::map<QString, std::vector<QString>> current_readers; //  Lectores activos
    std::vector<Resource> resources;

public:
    Semaphore(const std::vector<Resource>& res);
    bool tryAcquire(const QString& resource, const QString& pid, const QString& action_type) override;
    void release(const QString& resource, const QString& pid) override;
    bool isAvailable(const QString& resource) const override;
    void resetResources() override;
    int getAvailableCount(const QString& resource) const;
    
    bool hasActiveWriter(const QString& resource) const;
    int getActiveReaders(const QString& resource) const;
};

class SynchronizationSimulator {
public:
    static std::vector<SyncEvent> simulateSynchronization(
        const std::vector<Process>& processes,
        const std::vector<Resource>& resources,
        const std::vector<Action>& actions,
        SynchronizationMechanism* mechanism
    );
    
    static std::vector<SyncProcessState> getProcessStates(
        const std::vector<Process>& processes,
        const std::vector<SyncEvent>& events,
        int maxCycles
    );
};

class Synchronizer {
public:
    virtual ~Synchronizer() = default;
};

#endif