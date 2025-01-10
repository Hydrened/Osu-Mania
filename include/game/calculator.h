#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "game.h"
class Game;

class Calculator {
private:
    Game* game;

    int blockSize;
    int winWidth;
    int winHeight;

public:
    Calculator(Game* game);
    ~Calculator();

    H2DE_Pos convertToPx(BeatmapPos pos, BeatmapSize size);
    H2DE_Size convertToPx(BeatmapSize size);
};

#endif
