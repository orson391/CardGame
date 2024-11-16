#pragma once
#ifndef Game_h_
#define Game_h_
#include <iostream>
#include <SDL2/SDL.h>

class Game{
private:
    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;
public:
    Game();
    ~Game();
    bool IsRunning();
    void HandleEvents();
    void Render();
};

#endif