#ifndef BEATMAP_H
#define BEATMAP_H

#include "game.h"
class Game;

class Beatmap {
private:
    Game* game;
    std::filesystem::path difficulty;

    std::vector<Note> notes;
    int startIndexObj = 0;

    Uint32 delay;

    void loadSong();
    void loadNotes();

public:
    Beatmap(Game* game, std::filesystem::path difficulty);
    ~Beatmap();

    void update();
    void render();
};

#endif
