// scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "utils.h"

void fifo(std::vector<Process>& processes);
void sjf(std::vector<Process>& processes);

void displayGanttChart(const std::vector<Process>& processes);


#endif