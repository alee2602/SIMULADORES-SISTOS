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
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

// ==============================
// DATA STRUCTURES
// ==============================
struct Process {
    string pid;
    int burst_time;
    int arrival_time;
    int priority;

    // To obtain metrics 
    int start_time = -1;
    int finish_time = -1;
    int waiting_time = -1;
};

struct Action {
    string pid;
    string type;     // Whether it's READ or WRITE
    string resource;
    int cycle;
};

struct Resource {
    string name;
    int counter;
};

// ==============================
// FILE LOADERS
// ==============================
vector<Process> loadProcesses(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;

        istringstream ss(line);
        string pid, bt_str, at_str, prio_str;

        if (getline(ss, pid, ',') &&
            getline(ss, bt_str, ',') &&
            getline(ss, at_str, ',') &&
            getline(ss, prio_str)) {

            // Remove spaces
            pid.erase(remove(pid.begin(), pid.end(), ' '), pid.end());
            bt_str.erase(remove(bt_str.begin(), bt_str.end(), ' '), bt_str.end());
            at_str.erase(remove(at_str.begin(), at_str.end(), ' '), at_str.end());
            prio_str.erase(remove(prio_str.begin(), prio_str.end(), ' '), prio_str.end());

            // Convert and push
            int bt = stoi(bt_str);
            int at = stoi(at_str);
            int prio = stoi(prio_str);

            processes.push_back({pid, bt, at, prio});
        }
    }

    return processes;
}

vector<Resource> loadResources(const string& filename) {
    vector<Resource> resources;
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        istringstream ss(line);
        string name;
        int count;
        char sep;
        if (ss >> name >> sep >> count) {
            resources.push_back({name, count});
        }
    }
    return resources;
}

vector<Action> loadActions(const string& filename) {
    vector<Action> actions;
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        istringstream ss(line);
        string pid, type, resource;
        int cycle;
        char sep;
        if (ss >> pid >> sep >> type >> sep >> resource >> sep >> cycle) {
            actions.push_back({pid, type, resource, cycle});
        }
    }
    return actions;
}

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

// ==============================
// Timeline (GUI)
// ==============================

// Helper to render text
void renderText(SDL_Renderer* renderer, TTF_Font* font, const string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Generates up to 20 distinct colors
SDL_Color getColor(int index) {
    SDL_Color colors[20] = {
        {255, 99, 132, 255}, {54, 162, 235, 255}, {255, 206, 86, 255}, {75, 192, 192, 255},
        {153, 102, 255, 255}, {255, 159, 64, 255}, {199, 199, 199, 255}, {83, 102, 255, 255},
        {255, 140, 148, 255}, {100, 221, 23, 255}, {174, 234, 0, 255}, {0, 200, 83, 255},
        {0, 188, 212, 255}, {0, 145, 234, 255}, {126, 87, 194, 255}, {213, 0, 249, 255},
        {255, 109, 0, 255}, {121, 85, 72, 255}, {158, 158, 158, 255}, {96, 125, 139, 255}
    };
    return colors[index % 20];
}

void drawTimelineSDL(const vector<Process>& processes) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        cout << "SDL Error: " << SDL_GetError() << endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow("Scheduling Timeline", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1300, 300, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 14);
    if (!font) {
        cout << "Failed to load font\n";
        return;
    }

    const int blockW = 40, blockH = 60, spacing = 5;
    const int startX = 50, startY = 100;
    int scrollX = 0;
    int scrollStep = 40;

    int maxTime = 0;
    for (auto& p : processes)
        if (p.finish_time > maxTime)
            maxTime = p.finish_time;

    map<string, SDL_Color> colorMap;
    int colorIndex = 0;
    for (const auto& p : processes) {
        if (colorMap.find(p.pid) == colorMap.end()) {
            colorMap[p.pid] = getColor(colorIndex++);
        }
    }

    int currentCycle = 0;
    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_LEFT)
                    scrollX = max(0, scrollX - scrollStep);
                else if (e.key.keysym.sym == SDLK_RIGHT)
                    scrollX += scrollStep;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (const auto& p : processes) {
            SDL_Color color = colorMap[p.pid];
            for (int t = p.start_time; t < p.finish_time && t <= currentCycle; ++t) {
                int x = startX + t * (blockW + spacing) - scrollX;
                SDL_Rect rect = {x, startY, blockW, blockH};
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
                SDL_RenderFillRect(renderer, &rect);
                renderText(renderer, font, p.pid, x + 8, startY + 20, {0, 0, 0, 255});
            }
        }

        for (int t = 0; t <= currentCycle; ++t) {
            int x = startX + t * (blockW + spacing) - scrollX;
            renderText(renderer, font, to_string(t), x + 10, startY + blockH + 10, {0, 0, 0, 255});
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(500); // Delay between cycles
        if (currentCycle < maxTime) currentCycle++;
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

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
            auto processes = loadProcesses("processes.txt");
            if (processes.empty()) {
                cout << "[ERROR] No processes found in 'processes.txt'.\n";
                continue;
            }

            fifo(processes);
            drawTimelineSDL(processes);

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