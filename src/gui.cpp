#include "gui.h"
#include "utils.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>
#include <algorithm>
#include <iostream> 

// ==============================
// Timeline (GUI)
// ==============================


void drawTimelineSDL(const std::vector<Process>& processes) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        std::cout << "SDL Error: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow("Scheduling Timeline", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1300, 300, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 14);
    if (!font) {
        std::cout << "Failed to load font\n";
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

    std::map<std::string, SDL_Color> colorMap;
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
                    scrollX = std::max(0, scrollX - scrollStep);
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
            renderText(renderer, font, std::to_string(t), x + 10, startY + blockH + 10, {0, 0, 0, 255});
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(500);
        if (currentCycle < maxTime) currentCycle++;
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
