#ifndef LOADER_H
#define LOADER_H

#include "utils.h"
#include <vector>
#include <QString>

std::vector<Process> loadProcesses(const QString& filename);
std::vector<Resource> loadResources(const QString& filename);
std::vector<Action> loadActions(const QString& filename);

#endif 