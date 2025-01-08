#ifndef BEATMAP_H
#define BEATMAP_H

#include <cmath>
#include <unordered_set>
#include "game.h"
class Game;

class Beatmap {
private:
    Game* game;
    std::filesystem::path beatmap;

    int speed;
    std::string song;
    Uint32 delay;

    std::vector<Note*> notes;
    std::unordered_set<Note*> pressedNotes;

    std::vector<bool> keysDown = { false, false, false, false };
    std::vector<int> keyPressedThisFrame;
    std::vector<int> keyReleasedThisFrame;

    void loadSettings();
    void loadSong();
    void loadNotes();

    Note* getClosestNote(int time, int key);
    Judgment getJudgment(int delay);

public:
    Beatmap(Game* game, std::filesystem::path beatmap);
    ~Beatmap();

    void update();
    void render();

    void inputDown(int key);
    void inputUp(int key);
};

#endif
