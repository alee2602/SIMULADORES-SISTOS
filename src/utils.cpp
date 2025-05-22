#include "utils.h"

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

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
