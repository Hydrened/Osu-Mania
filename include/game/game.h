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
#include "calculator.h"
#include "menu.h"
#include "beatmap.h"
class Calculator;
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

    Calculator* calculator = nullptr;
    Settings* settings = nullptr;
    Menu* menu = nullptr;
    Beatmap* beatmap = nullptr;
    GameState state = MAIN_MENU;

    std::vector<SDL_Keycode> keys;

    void createWindow();
    void initMainFolder();
    void loadKeys();
    void importBeatmap(char* file);

public:
    Game(int argc, char** argv);
    ~Game();

    void run();
    void quit();
    
    void handleEvents(SDL_Event event);
    void update();
    void render();

    void openMenu();
    void closeMenu();

    void openBeatmap(std::filesystem::path beatmap);
    void closeBeatmap();

    static void delay(int ms, std::function<void()> callback);

    GameData* getData() const;
    H2DE_Engine* getEngine() const;
    Calculator* getCalculator() const;
    int getFPS() const;
    GameState getState() const;
    Settings* getSettings() const;
    std::filesystem::path getMainFolder();
    void getWinSize(int* width, int* height);

    void setState(GameState state);
};

#endif
