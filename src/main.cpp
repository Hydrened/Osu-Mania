#include "game.h"
#include <windows.h>

int main(int argc, char** argv) {
    try {
        Game* game = new Game(argc, argv);
        game->run();
        delete game;
    } catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}
