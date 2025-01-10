#include "calculator.h"

// INIT
Calculator::Calculator(Game* g) : game(g) {
    g->getWinSize(&winWidth, &winHeight);
    blockSize = static_cast<int>(winWidth / BLOCKS_ON_WIDTH);
}

// CLEANUP
Calculator::~Calculator() {
    
}

// CONVERT
H2DE_Pos Calculator::convertToPx(BeatmapPos pos, BeatmapSize size) {
    static float hitZone = game->getData()->offsets->hitZone;
    return {
        static_cast<int>((winWidth - (4 * blockSize)) / 2 + (pos.x * blockSize)),
        static_cast<int>(winHeight - ((pos.y + size.h + hitZone) * blockSize))
    };
}

H2DE_Size Calculator::convertToPx(BeatmapSize size) {
    return {
        static_cast<int>(size.w * blockSize),
        static_cast<int>(size.h * blockSize)
    };
}
