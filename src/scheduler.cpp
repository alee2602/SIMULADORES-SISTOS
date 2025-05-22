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
void fifo(vector<Process> &processes)
{
    if (processes.empty())
    {
        cout << "[ERROR] No processes loaded. Please check 'processes.txt'.\n";
        return;
    }

    sort(processes.begin(), processes.end(), [](Process a, Process b)
        { return a.arrival_time < b.arrival_time; });

    int current_time = 0;
    for (auto &p : processes)
    {
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
    for (auto &p : processes)
        avg_wait += p.waiting_time;
    cout << fixed << setprecision(2);
    cout << "Average Waiting Time: " << avg_wait / processes.size() << endl;

    // Summary Table
    cout << "\nProcess Summary:\n";
    cout << left << setw(8) << "PID"
        << setw(12) << "Start"
        << setw(12) << "Finish"
        << setw(12) << "Waiting" << endl;

    for (const auto &p : processes)
    {
        cout << left << setw(8) << p.pid
            << setw(12) << p.start_time
            << setw(12) << p.finish_time
            << setw(12) << p.waiting_time << endl;
    }
}

void sjf(vector<Process> &processes)
{
    if (processes.empty())
    {
        cout << "[ERROR] No processes loaded. Please check 'processes.txt'.\n";
        return;
    }

    int current_time = 0;
    vector<Process> ready_queue;
    vector<Process> remaining = processes;
    vector<Process> executed;

    while (!remaining.empty() || !ready_queue.empty())
    {
        // Add processes that are in the ready queue 
        for (auto it = remaining.begin(); it != remaining.end();)
        {
            if (it->arrival_time <= current_time)
            {
                ready_queue.push_back(*it);
                it = remaining.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if (!ready_queue.empty())
        {
            // Choose the process with the shortest burst time
            auto shortest = min_element(ready_queue.begin(), ready_queue.end(),[](const Process &a, const Process &b)
            {return a.burst_time < b.burst_time;});

            Process current = *shortest;
            ready_queue.erase(shortest);

            current.start_time = current_time;
            current.finish_time = current.start_time + current.burst_time;
            current.waiting_time = current.start_time - current.arrival_time;

            current_time = current.finish_time;
            executed.push_back(current);
        }
        else
        {
            current_time++;
        }
    }
    processes = executed;

    // Show results
    double total_waiting = 0;
    cout << fixed << setprecision(2);
    for (const auto &p : executed)
    {
        total_waiting += p.waiting_time;
        cout << "Process " << p.pid << " executed from " << p.start_time << " to " << p.finish_time << endl;
    }

    cout << "Average Waiting Time: " << total_waiting / executed.size() << endl;

    // Table summary
    cout << "\nProcess Summary:\n";
    cout << left << setw(8) << "PID"
        << setw(12) << "Start"
        << setw(12) << "Finish"
        << setw(12) << "Waiting" << endl;

    for (const auto &p : executed)
    {
        cout << left << setw(8) << p.pid
            << setw(12) << p.start_time
            << setw(12) << p.finish_time
            << setw(12) << p.waiting_time << endl;
    }

    processes = executed;
}

// ==============================
// GANTT CHART (terminal-only)
// ==============================
void displayGanttChart(const vector<Process> &processes)
{
    cout << "\nGantt Chart:\n";
    for (auto &p : processes)
    {
        cout << p.pid << ": ";
        for (int i = 0; i < p.start_time; ++i)
            cout << " ";
        for (int i = 0; i < p.burst_time; ++i)
            cout << "#";
        cout << "\n";
    }
}