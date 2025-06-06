#include "synchronizer.h"
#include <algorithm>
#include <map>
#include <set>

MutexLock::MutexLock(const std::vector<Resource>& res) : resources(res) {
    resetResources();
}

bool MutexLock::tryAcquire(const QString& resource, const QString& pid, const QString& action_type) {
    if (isAvailable(resource)) {
        resource_owners[resource] = pid;
        return true;
    }
    return false;
}

void MutexLock::release(const QString& resource, const QString& pid) {
    if (resource_owners.find(resource) != resource_owners.end() && 
        resource_owners[resource] == pid) {
        resource_owners.erase(resource);
    }
}

bool MutexLock::isAvailable(const QString& resource) const {
    return resource_owners.find(resource) == resource_owners.end();
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
    
    std::map<QString, std::vector<Action>> waiting_processes;
    std::map<QString, Action> active_processes; 
    
    int max_cycle = 0;
    if (!sorted_actions.empty()) {
        max_cycle = sorted_actions.back().cycle;
    }
    
    for (int current_cycle = 0; current_cycle <= max_cycle + 10; current_cycle++) {
        std::vector<QString> to_remove;
        for (auto& [pid, action] : active_processes) {
            mechanism->release(action.resource, pid);
            events.push_back(SyncEvent(pid, action.resource, action.type, 
                                     current_cycle, ProcessState::ACCESSED, 
                                     process_colors[pid]));
            to_remove.push_back(pid);
        }
        for (const auto& pid : to_remove) {
            active_processes.erase(pid);
        }

        std::vector<std::pair<QString, Action>> actions_this_cycle;

        for (const auto& action : sorted_actions) {
            if (action.cycle == current_cycle) {
                actions_this_cycle.emplace_back(action.pid, action);
            }
        }
        for (auto& [pid, waiting_actions] : waiting_processes) {
            for (const auto& action : waiting_actions) {
                actions_this_cycle.emplace_back(pid, action);
            }
        }
        waiting_processes.clear();

        if (dynamic_cast<MutexLock*>(mechanism)) {
            std::set<QString> recursos_ocupados;
            for (const auto& [pid, action] : actions_this_cycle) {
                if (active_processes.count(pid)) continue;
                if (recursos_ocupados.count(action.resource)) {
                    events.push_back(SyncEvent(pid, action.resource, action.type,
                                         current_cycle, ProcessState::WAITING,
                                         process_colors[pid]));
                    waiting_processes[pid].push_back(action);
                    continue;
                }
                if (mechanism->tryAcquire(action.resource, pid, action.type)) {
                    active_processes[pid] = action;
                    recursos_ocupados.insert(action.resource);
                } else {
                    events.push_back(SyncEvent(pid, action.resource, action.type,
                                         current_cycle, ProcessState::WAITING,
                                         process_colors[pid]));
                    waiting_processes[pid].push_back(action);
                }
            }
        } else if (auto sem = dynamic_cast<Semaphore*>(mechanism)) {
            std::map<QString, int> cupos_restantes;
            for (const auto& resource : resources) {
                cupos_restantes[resource.name] = sem->getAvailableCount(resource.name);
            }

            std::vector<std::pair<QString, Action>> waiting_fifo;
            for (auto& [pid, waiting_actions] : waiting_processes) {
                for (const auto& action : waiting_actions) {
                    waiting_fifo.emplace_back(pid, action);
                }
            }
            for (const auto& [pid, action] : actions_this_cycle) {
                if (waiting_processes.find(pid) == waiting_processes.end())
                    waiting_fifo.emplace_back(pid, action);
            }
            waiting_processes.clear();

            for (const auto& [pid, action] : waiting_fifo) {
                if (active_processes.count(pid)) continue;
                if (cupos_restantes[action.resource] > 0 && sem->tryAcquire(action.resource, pid, action.type)) {
                    active_processes[pid] = action;
                    cupos_restantes[action.resource]--;
                } else {
                    events.push_back(SyncEvent(pid, action.resource, action.type,
                                         current_cycle, ProcessState::WAITING,
                                         process_colors[pid]));
                    waiting_processes[pid].push_back(action);
                }
            }
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