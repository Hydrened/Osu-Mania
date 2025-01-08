#ifndef MENU_H
#define MENU_H

#include "game.h"
class Game;

class Menu {
private:
    Game* game;

    std::unordered_map<int, std::unordered_map<std::string, std::filesystem::path>> beatmaps;

public:
    Menu(Game* game);
    ~Menu();

    void loadBeatmaps();
    void openBeatmap(int id, std::string difficulty);
};

#endif
