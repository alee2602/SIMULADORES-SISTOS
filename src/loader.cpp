#include "loader.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

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