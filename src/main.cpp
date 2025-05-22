// All libraries that will be used for the project
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <map>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include "loader.h"
#include "scheduler.h"
#include "gui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

// ==============================
// MAIN MENU
// ==============================
void mainMenu() {
    int option;
    do {
        cout << "\n=== SIMULATOR ===\n";
        cout << "1. Scheduling Simulation\n";
        cout << "2. Synchronization Simulation (mutex/semaphore)\n";
        cout << "0. Exit\n";
        cout << "Select an option: ";
        cin >> option;

        if (option == 1) {
        int schedOption;
        do {
            cout << "\n-- SCHEDULING ALGORITHMS --\n";
            cout << "1. FIFO\n";
            cout << "2. SJF (Shortest Job First)\n";
            cout << "0. Back to main menu\n";
            cout << "Select an algorithm: ";
            cin >> schedOption;

            if (schedOption == 1) {
                auto processes = loadProcesses("../data/processes_5.txt");
                if (processes.empty()) {
                    cout << "[ERROR] No processes found.\n";
                    continue;
                }
                fifo(processes);
                drawTimelineSDL(processes);
            }
            else if (schedOption == 2) {
                auto processes = loadProcesses("../data/processes_5.txt");
                if (processes.empty()) {
                    cout << "[ERROR] No processes found.\n";
                    continue;
                }
                sjf(processes);
                drawTimelineSDL(processes);
            }

        } while (schedOption != 0);
    }

    else if (option == 2) {
        cout << "[!] Synchronization module not yet implemented.\n";
    }

} while (option != 0);

}

// ==============================
// EXECUTE MAIN
// ==============================
int main() {
    mainMenu();
    return 0;
}