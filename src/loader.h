// loader.h
#ifndef LOADER_H
#define LOADER_H

#include <vector>
#include "utils.h"

std::vector<Process> loadProcesses(const std::string& filename);
std::vector<Resource> loadResources(const std::string& filename);
std::vector<Action> loadActions(const std::string& filename);

#endif
