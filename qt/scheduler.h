#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "utils.h"
#include <vector>

class SchedulingAlgorithms {
public:
    static std::vector<ExecutionSlice> runFIFO(std::vector<Process>& processes);
    static std::vector<ExecutionSlice> runSJF(std::vector<Process>& processes); 
    static std::vector<ExecutionSlice> runSRT(std::vector<Process>& processes);
    static std::vector<ExecutionSlice> runRoundRobin(std::vector<Process>& processes, int quantum);
    static std::vector<ExecutionSlice> runPriority(std::vector<Process>& processes, bool agingEnabled, int agingInterval = 5);

    static double calculateAverageWaitingTime(const std::vector<Process>& processes);
    static double calculateAverageCompletionTime(const std::vector<Process>& processes);
};

#endif