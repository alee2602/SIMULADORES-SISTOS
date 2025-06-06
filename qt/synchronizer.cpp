#include "synchronizer.h"
#include <algorithm>
#include <map>

// ==============================
// MUTEX LOCK IMPLEMENTATION
// ==============================

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

// ==============================
// SEMAPHORE IMPLEMENTATION
// ==============================

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

// ==============================
// SYNCHRONIZATION SIMULATOR
// ==============================

std::vector<SyncEvent> SynchronizationSimulator::simulateSynchronization(
    const std::vector<Process>& processes,
    const std::vector<Resource>& resources,
    const std::vector<Action>& actions,
    SynchronizationMechanism* mechanism) {
    
    std::vector<SyncEvent> events;
    std::map<QString, QColor> process_colors;
    
    // Initialize process colors
    for (const auto& process : processes) {
        process_colors[process.pid] = process.color;
    }
    
    // Sort actions by cycle
    std::vector<Action> sorted_actions = actions;
    std::sort(sorted_actions.begin(), sorted_actions.end(),
        [](const Action& a, const Action& b) {
            return a.cycle < b.cycle;
        });
    
    mechanism->resetResources();
    
    // Track waiting processes
    std::map<QString, std::vector<Action>> waiting_processes;
    std::map<QString, Action> active_processes; 
    
    int max_cycle = 0;
    if (!sorted_actions.empty()) {
        max_cycle = sorted_actions.back().cycle;
    }
    
    for (int current_cycle = 0; current_cycle <= max_cycle + 10; current_cycle++) {
        // Release resources for completed actions
        std::vector<QString> to_remove;
        for (auto& [pid, action] : active_processes) {
            // Each action takes 1 cycle, so release after 1 cycle
            mechanism->release(action.resource, pid);
            events.push_back(SyncEvent(pid, action.resource, action.type, 
                                     current_cycle, ProcessState::ACCESSED, 
                                     process_colors[pid]));
            to_remove.push_back(pid);
        }
        
        for (const auto& pid : to_remove) {
            active_processes.erase(pid);
        }
        
        // Try to satisfy waiting processes
        std::map<QString, std::vector<Action>> still_waiting;
        for (auto& [pid, waiting_actions] : waiting_processes) {
            std::vector<Action> remaining_actions;
            
            for (const auto& action : waiting_actions) {
                if (mechanism->tryAcquire(action.resource, pid, action.type)) {
                    active_processes[pid] = action;
                } else {
                    events.push_back(SyncEvent(pid, action.resource, action.type,
                                             current_cycle, ProcessState::WAITING,
                                             process_colors[pid]));
                    remaining_actions.push_back(action);
                }
            }
            
            if (!remaining_actions.empty()) {
                still_waiting[pid] = remaining_actions;
            }
        }
        waiting_processes = still_waiting;
        
        // Process new actions for this cycle
        for (const auto& action : sorted_actions) {
            if (action.cycle == current_cycle) {
                if (mechanism->tryAcquire(action.resource, action.pid, action.type)) {
                    active_processes[action.pid] = action;
                } else {
                    events.push_back(SyncEvent(action.pid, action.resource, action.type,
                                             current_cycle, ProcessState::WAITING,
                                             process_colors[action.pid]));
                    waiting_processes[action.pid].push_back(action);
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
    
    // Initialize states
    for (const auto& process : processes) {
        current_states[process.pid] = SyncProcessState(process.pid, process.color);
    }
    
    // Process events to determine states at each cycle
    for (int cycle = 0; cycle <= maxCycles; cycle++) {
        for (auto& [pid, state] : current_states) {
            state.current_state = ProcessState::WAITING;
            state.waiting_for_resource = "";
        }
        
        // Apply events for this cycle
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
        
        // Add current states to result
        for (const auto& [pid, state] : current_states) {
            states.push_back(state);
        }
    }
    
    return states;
}