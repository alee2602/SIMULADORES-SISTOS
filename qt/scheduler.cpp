#include "scheduler.h"
#include <algorithm>
#include <queue>
#include <map>

std::vector<ExecutionSlice> SchedulingAlgorithms::runFIFO(std::vector<Process>& processes) {
    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    std::vector<ExecutionSlice> timeline;
    int currentTime = 0;

    for (auto& p : processes) {
        if (currentTime < p.arrival_time) {
            currentTime = p.arrival_time;
        }
        p.start_time = currentTime;
        p.finish_time = currentTime + p.burst_time;
        p.waiting_time = p.start_time - p.arrival_time;
        
        timeline.push_back(ExecutionSlice(p.pid, currentTime, p.burst_time, p.color));
        currentTime += p.burst_time;
    }

    return timeline;
}

std::vector<ExecutionSlice> SchedulingAlgorithms::runSJF(std::vector<Process>& processes) {
    std::vector<ExecutionSlice> timeline;
    std::vector<Process> remaining = processes;
    std::vector<Process> ready_queue;
    std::vector<Process> executed;
    int currentTime = 0;

    while (!remaining.empty() || !ready_queue.empty()) {
        // Add arrived processes to ready queue
        for (auto it = remaining.begin(); it != remaining.end();) {
            if (it->arrival_time <= currentTime) {
                ready_queue.push_back(*it);
                it = remaining.erase(it);
            } else {
                ++it;
            }
        }

        if (!ready_queue.empty()) {
            // Find shortest job
            auto shortest = std::min_element(ready_queue.begin(), ready_queue.end(),
                [](const Process& a, const Process& b) {
                    return a.burst_time < b.burst_time;
                });

            Process current = *shortest;
            ready_queue.erase(shortest);

            current.start_time = currentTime;
            current.finish_time = currentTime + current.burst_time;
            current.waiting_time = current.start_time - current.arrival_time;

            timeline.push_back(ExecutionSlice(current.pid, currentTime, current.burst_time, current.color));
            
            currentTime = current.finish_time;
            executed.push_back(current);
        } else {
            currentTime++;
        }
    }

    processes = executed;
    return timeline;
}

std::vector<ExecutionSlice> SchedulingAlgorithms::runSRT(std::vector<Process>& processes) {
    std::vector<ExecutionSlice> timeline;
    std::vector<Process> remaining = processes;
    std::vector<Process> ready_queue;
    std::map<QString, int> remaining_bt;
    std::map<QString, int> start_times;
    std::vector<Process> executed;
    int currentTime = 0;

    // Initialize remaining burst times
    for (const auto& p : processes) {
        remaining_bt[p.pid] = p.burst_time;
    }

    while (!ready_queue.empty() || !remaining.empty()) {
        // Add arrived processes
        for (auto it = remaining.begin(); it != remaining.end();) {
            if (it->arrival_time <= currentTime) {
                ready_queue.push_back(*it);
                it = remaining.erase(it);
            } else {
                ++it;
            }
        }

        if (!ready_queue.empty()) {
            // Find shortest remaining time
            auto shortest = std::min_element(ready_queue.begin(), ready_queue.end(),
                [&remaining_bt](const Process& a, const Process& b) {
                    return remaining_bt[a.pid] < remaining_bt[b.pid];
                });

            Process current = *shortest;
            
            // Record start time
            if (start_times.find(current.pid) == start_times.end()) {
                start_times[current.pid] = currentTime;
            }

            // Execute for 1 time unit
            timeline.push_back(ExecutionSlice(current.pid, currentTime, 1, current.color));
            currentTime++;
            remaining_bt[current.pid]--;

            // Check if process is complete
            if (remaining_bt[current.pid] == 0) {
                current.start_time = start_times[current.pid];
                current.finish_time = currentTime;
                current.waiting_time = current.finish_time - current.arrival_time - current.burst_time;
                executed.push_back(current);
                ready_queue.erase(shortest);
            }
        } else {
            currentTime++;
        }
    }

    processes = executed;
    return timeline;
}

std::vector<ExecutionSlice> SchedulingAlgorithms::runRoundRobin(std::vector<Process>& processes, int quantum) {
    std::vector<ExecutionSlice> timeline;
    std::queue<Process> ready_queue;
    std::vector<Process> remaining = processes;
    std::map<QString, int> remaining_bt;
    std::map<QString, int> start_times;
    std::vector<Process> executed;
    int currentTime = 0;

    for (auto& p : processes) {
        remaining_bt[p.pid] = p.burst_time;
    }

    while (!ready_queue.empty() || !remaining.empty()) {
        // Add arrived processes
        for (auto it = remaining.begin(); it != remaining.end();) {
            if (it->arrival_time <= currentTime) {
                ready_queue.push(*it);
                it = remaining.erase(it);
            } else {
                ++it;
            }
        }

        if (!ready_queue.empty()) {
            Process current = ready_queue.front();
            ready_queue.pop();

            if (start_times.find(current.pid) == start_times.end()) {
                start_times[current.pid] = currentTime;
            }

            int exec_time = std::min(quantum, remaining_bt[current.pid]);
            timeline.push_back(ExecutionSlice(current.pid, currentTime, exec_time, current.color));

            currentTime += exec_time;
            remaining_bt[current.pid] -= exec_time;

            // Add new arrivals
            for (auto it = remaining.begin(); it != remaining.end();) {
                if (it->arrival_time <= currentTime) {
                    ready_queue.push(*it);
                    it = remaining.erase(it);
                } else {
                    ++it;
                }
            }

            if (remaining_bt[current.pid] > 0) {
                ready_queue.push(current);
            } else {
                current.finish_time = currentTime;
                current.waiting_time = (current.finish_time - current.arrival_time) - current.burst_time;
                current.start_time = start_times[current.pid];
                executed.push_back(current);
            }
        } else {
            currentTime++;
        }
    }

    processes = executed;
    return timeline;
}

std::vector<ExecutionSlice> SchedulingAlgorithms::runPriority(std::vector<Process>& processes, bool agingEnabled, int agingInterval) {
    std::vector<ExecutionSlice> timeline;
    std::vector<Process> remaining = processes;
    std::vector<Process> ready_queue;
    std::map<QString, int> wait_time;
    std::vector<Process> executed;
    int currentTime = 0;

    // Inicializar tiempos de espera
    for (const auto& p : processes) {
        wait_time[p.pid] = 0;
    }

    while (!remaining.empty() || !ready_queue.empty()) {
        // Añadir procesos que han llegado
        for (auto it = remaining.begin(); it != remaining.end();) {
            if (it->arrival_time <= currentTime) {
                ready_queue.push_back(*it);
                it = remaining.erase(it);
            } else {
                ++it;
            }
        }

        // Si hay procesos listos
        if (!ready_queue.empty()) {
            if (agingEnabled) {
                for (auto& p : ready_queue) {
                    wait_time[p.pid]++;
                }
                for (size_t i = 0; i < ready_queue.size(); ++i) {
                    const QString& pid = ready_queue[i].pid;
                    if (wait_time[pid] % agingInterval == 0 && ready_queue[i].priority > 1) {
                        ready_queue[i].priority--;
                    }
                }
            }

            // Seleccionar el de mayor prioridad
            auto highest_priority = std::min_element(ready_queue.begin(), ready_queue.end(),
                [](const Process& a, const Process& b) {
                    return a.priority < b.priority;
                });

            Process current = *highest_priority;
            ready_queue.erase(highest_priority);

            current.start_time = currentTime;
            current.finish_time = currentTime + current.burst_time;
            current.waiting_time = current.start_time - current.arrival_time;

            timeline.push_back(ExecutionSlice(current.pid, currentTime, current.burst_time, current.color));

            currentTime = current.finish_time;
            executed.push_back(current);
            wait_time.erase(current.pid); 
        } else {
            currentTime++;
        }
    }

    processes = executed;
    return timeline;
}

double SchedulingAlgorithms::calculateAverageWaitingTime(const std::vector<Process>& processes) {
    if (processes.empty()) return 0.0;
    
    double total = 0.0;
    for (const auto& p : processes) {
        total += p.waiting_time;
    }
    return total / processes.size();
}

double SchedulingAlgorithms::calculateAverageCompletionTime(const std::vector<Process>& processes) {
    if (processes.empty()) return 0.0;
    
    double total = 0.0;
    for (const auto& p : processes) {
        total += p.finish_time;
    }
    return total / processes.size();
}