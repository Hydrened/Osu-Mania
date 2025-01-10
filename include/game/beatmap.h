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

    H2DE_TimelineManager* tm = H2DE_CreateTimelineManager();

    int speed;
    std::string song;
    std::string bgPicture;
    Uint8 bgOpacity;
    Uint32 accDelay;

    std::vector<Note*> notes;
    std::unordered_set<Note*> pressedNotes;
    std::vector<Note*> slidersHolding;
    std::unordered_map<Note*, Judgment> sliderJudgments;
    std::unordered_map<Judgment, int> judgments;

    Judgment* thisFrameJudgment = nullptr;
    Judgment* currentJudgment = nullptr;
    float currentJudgmentScale = 1.0f;

    std::vector<bool> keysDown = { false, false, false, false };
    std::vector<int> keyPressedThisFrame;
    std::vector<int> keyReleasedThisFrame;

    void loadSettings();
    void loadSong();
    void loadBgPicture();
    void loadNotes();

    void addJudgment(Judgment judgment);
    Note* getClosestNote(int time, int key);
    Judgment getJudgment(int delay);

    void renderBackground();
    void renderColumn();
    void renderNotes();
    void renderJudgment();

public:
    Beatmap(Game* game, std::filesystem::path beatmap);
    ~Beatmap();

    void update();
    void render();

    void inputDown(int key);
    void inputUp(int key);
};

#endif
