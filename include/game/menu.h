#ifndef MENU_H
#define MENU_H

#include "game.h"
class Game;

class Menu {
private:
    Game* game;

public:
    Menu(Game* game);
    ~Menu();
};

#endif
