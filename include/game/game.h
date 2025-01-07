#ifndef GAME_H
#define GAME_H

#include <H2DE/H2DE.h>
#include <filesystem>
#include <functional>
#include <iostream>
#include <thread>
#include "miniz/miniz.h"
#include "utils.h"
#include "data.h"
#include "functions.h"
#include "settings.h"
#include "menu.h"
#include "beatmap.h"
class Menu;
class Beatmap;

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    H2DE_Engine* engine;
    int FPS = 60;
    bool isRunning = true;
    std::filesystem::path mainFolder;
    GameData* data = new GameData();
    // Calculator* calculator;

    Settings* settings = nullptr;
    Menu* menu = nullptr;
    Beatmap* beatmap = nullptr;
    GameState state = MAIN_MENU;

    void createWindow();
    void initMainFolder();

    void openMenu();
    void closeMenu();
    
    void loadBeatmap(char* file);
    void openBeatmap(std::filesystem::path difficulty);
    void closeBeatmap();

public:
    Game(int argc, char** argv);
    ~Game();

    void run();
    void quit();
    
    void handleEvents(SDL_Event event);
    void update();
    void render();

    static void delay(int ms, std::function<void()> callback);

    GameData* getData() const;
    H2DE_Engine* getEngine() const;
    int getFPS() const;
    // Calculator* getCalculator() const;
    // Beatmap* getBeatmap() const;
    // GameState getState() const;
    std::filesystem::path getMainFolder();

    void setState(GameState state);
};

#endif
