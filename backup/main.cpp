#define SDL_MAIN_HANDLED
#include "Game.h"

int main(int argc, char* argv[]) {
    Game* game = new Game();
    while(game->IsRunning()){
        game->HandleEvents();
        game->Render();
    }
    return 0;
}
