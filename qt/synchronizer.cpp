#include "synchronizer.h"
#include <algorithm>
#include <map>
#include <set>
#include <queue>

MutexLock::MutexLock(const std::vector<Resource>& res) : resources(res) {
    resetResources();
}

bool MutexLock::tryAcquire(const QString& resource, const QString& pid, const QString& action_type) {
    // Solo un proceso por recurso, sin importar READ/WRITE
    if (resource_owners.find(resource) == resource_owners.end()) {
        // Recurso libre, se asigna
        resource_owners[resource] = pid;
        return true;
    }
    // Recurso ocupado, debe esperar
    return false;
}

void MutexLock::release(const QString& resource, const QString& pid) {
    // Liberar si es el dueño actual
    if (resource_owners.find(resource) != resource_owners.end() && 
        resource_owners[resource] == pid) {
        resource_owners.erase(resource);
    }
}

bool MutexLock::isAvailable(const QString& resource) const {
    return resource_owners.find(resource) == resource_owners.end();
}

bool MutexLock::hasWriter(const QString& resource) const {
    return resource_owners.find(resource) != resource_owners.end();
}

bool MutexLock::hasReaders(const QString& resource) const {
    return false; 
}

void MutexLock::resetResources() {
    resource_owners.clear();
}

Semaphore::Semaphore(const std::vector<Resource>& res) : resources(res) {
    resetResources();
}

bool Semaphore::tryAcquire(const QString& resource, const QString& pid, const QString& action_type) {
    if (resource_counts[resource] > 0) {
        resource_counts[resource]--;
        resource_holders[resource].push_back(pid);
        return true;
    }
    return false;
}

void Semaphore::release(const QString& resource, const QString& pid) {
    auto& holders = resource_holders[resource];
    auto it = std::find(holders.begin(), holders.end(), pid);
    if (it != holders.end()) {
        holders.erase(it);
        resource_counts[resource]++;
    }
}

bool Semaphore::isAvailable(const QString& resource) const {
    auto it = resource_counts.find(resource);
    return (it != resource_counts.end() && it->second > 0);
}

void Semaphore::resetResources() {
    resource_counts.clear();
    resource_holders.clear();
    
    for (const auto& resource : resources) {
        resource_counts[resource.name] = resource.count;
        resource_holders[resource.name] = std::vector<QString>();
    }
}

int Semaphore::getAvailableCount(const QString& resource) const {
    auto it = resource_counts.find(resource);
    return (it != resource_counts.end()) ? it->second : 0;
}

std::vector<SyncEvent> SynchronizationSimulator::simulateSynchronization(
    const std::vector<Process>& processes,
    const std::vector<Resource>& resources,
    const std::vector<Action>& actions,
    SynchronizationMechanism* mechanism) {
    
    std::vector<SyncEvent> events;
    std::map<QString, QColor> process_colors;
    
    for (const auto& process : processes) {
        process_colors[process.pid] = process.color;
    }
    
    std::vector<Action> sorted_actions = actions;
    std::sort(sorted_actions.begin(), sorted_actions.end(),
        [](const Action& a, const Action& b) {
            return a.cycle < b.cycle;
        });
    
    mechanism->resetResources();
    
    std::queue<Action> waiting_queue;
    std::map<QString, Action> active_processes; 
    
    int max_cycle = 0;
    if (!sorted_actions.empty()) {
        max_cycle = sorted_actions.back().cycle;
    }
    
    for (int current_cycle = 0; current_cycle <= max_cycle + 5; current_cycle++) {
        
        // Liberar procesos que terminaron su ejecución
        std::vector<QString> to_remove;
        for (auto& [pid, action] : active_processes) {
            mechanism->release(action.resource, pid);
            to_remove.push_back(pid);
        }
        for (const auto& pid : to_remove) {
            active_processes.erase(pid);
        }

        // Agregar nuevas acciones de este ciclo a la cola FIFO
        for (const auto& action : sorted_actions) {
            if (action.cycle == current_cycle) {
                waiting_queue.push(action);
            }
        }

        // Procesar cola FIFO
        std::queue<Action> still_waiting;
        
        while (!waiting_queue.empty()) {
            Action current_action = waiting_queue.front();
            waiting_queue.pop();
            
            // Solo intentar si el proceso no está ya activo
            if (active_processes.count(current_action.pid) == 0) {
                
                if (mechanism->tryAcquire(current_action.resource, current_action.pid, current_action.type)) {
                    // Proceso obtiene recurso
                    active_processes[current_action.pid] = current_action;
                    events.push_back(SyncEvent(current_action.pid, current_action.resource, 
                                            current_action.type, current_cycle, 
                                            ProcessState::ACCESSED, 
                                            process_colors[current_action.pid]));
                } else {
                    // Recurso ocupado, va a seguir esperando
                    events.push_back(SyncEvent(current_action.pid, current_action.resource, 
                                            current_action.type, current_cycle, 
                                            ProcessState::WAITING, 
                                            process_colors[current_action.pid]));
                    still_waiting.push(current_action);
                }
            }
        }
        
        // Mantener orden FIFO
        waiting_queue = still_waiting;
        
        // Terminar si no hay más procesos activos ni esperando
        if (active_processes.empty() && waiting_queue.empty() && current_cycle > max_cycle) {
            break;
        }
    }
    
    return events;
}

std::vector<SyncProcessState> SynchronizationSimulator::getProcessStates(
    const std::vector<Process>& processes,
    const std::vector<SyncEvent>& events,
    int maxCycles) {
    
    std::vector<SyncProcessState> states;
    std::map<QString, SyncProcessState> current_states;
    
    for (const auto& process : processes) {
        current_states[process.pid] = SyncProcessState(process.pid, process.color);
    }
    
    for (int cycle = 0; cycle <= maxCycles; cycle++) {
        for (auto& [pid, state] : current_states) {
            state.current_state = ProcessState::WAITING;
            state.waiting_for_resource = "";
        }
        
        for (const auto& event : events) {
            if (event.cycle == cycle) {
                current_states[event.pid].current_state = event.state;
                if (event.state == ProcessState::WAITING) {
                    current_states[event.pid].waiting_for_resource = event.resource;
                    current_states[event.pid].cycles_waiting++;
                } else {
                    current_states[event.pid].cycles_waiting = 0;
                }
            }
        }
        
        for (const auto& [pid, state] : current_states) {
            states.push_back(state);
        }
    }
    
    return states;
}