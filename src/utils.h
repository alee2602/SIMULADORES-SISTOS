#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// ==============================
// DATA STRUCTURES
// ==============================

struct Process {
    std::string pid;
    int burst_time;
    int arrival_time;
    int priority;

    //Metrics
    int start_time = -1;
    int finish_time = -1;
    int waiting_time = -1;
};

struct Resource {
    std::string name;
    int counter;
};

struct Action {
    std::string pid;
    std::string type; // Whether it's READ or WRITE
    std::string resource;
    int cycle;
};

struct ExecutionSlice {
    std::string pid;
    int start;
    int duration;
};


// Obtener colores para los distintos procesos
SDL_Color getColor(int index);
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color);

#endif
