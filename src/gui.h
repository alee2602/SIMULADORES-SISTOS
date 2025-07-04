#ifndef GUI_H
#define GUI_H

#include <vector>
#include "utils.h"

void drawTimelineSDL(const std::vector<Process>& processes);  
void drawTimelineSDL(const std::vector<ExecutionSlice>& timeline);

#endif
