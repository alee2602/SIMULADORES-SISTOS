#include "scheduler.h"
#include "utils.h"

#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace std;


// ==============================
// SCHEDULING ALGORITHMS
// ==============================
void fifo(vector<Process>& processes) {
    if (processes.empty()) {
        cout << "[ERROR] No processes loaded. Please check 'processes.txt'.\n";
        return;
    }

    sort(processes.begin(), processes.end(), [](Process a, Process b) {
        return a.arrival_time < b.arrival_time;
    });

    int current_time = 0;
    for (auto& p : processes) {
        if (current_time < p.arrival_time)
            current_time = p.arrival_time;
        p.start_time = current_time;
        p.finish_time = current_time + p.burst_time;
        p.waiting_time = p.start_time - p.arrival_time;
        current_time += p.burst_time;

        cout << "Process " << p.pid << " executed from " << p.start_time << " to " << p.finish_time << endl;
    }

    // Metrics
    double avg_wait = 0;
    for (auto& p : processes) avg_wait += p.waiting_time;
    cout << fixed << setprecision(2);
    cout << "Average Waiting Time: " << avg_wait / processes.size() << endl;

    // Summary Table
    cout << "\nProcess Summary:\n";
    cout << left << setw(8) << "PID"
        << setw(12) << "Start"
        << setw(12) << "Finish"
        << setw(12) << "Waiting" << endl;

    for (const auto& p : processes) {
        cout << left << setw(8) << p.pid
            << setw(12) << p.start_time
            << setw(12) << p.finish_time
            << setw(12) << p.waiting_time << endl;
    }
}

// ==============================
// GANTT CHART (terminal-only)
// ==============================
void displayGanttChart(const vector<Process>& processes) {
    cout << "\nGantt Chart:\n";
    for (auto& p : processes) {
        cout << p.pid << ": ";
        for (int i = 0; i < p.start_time; ++i) cout << " ";
        for (int i = 0; i < p.burst_time; ++i) cout << "#";
        cout << "\n";
    }
}