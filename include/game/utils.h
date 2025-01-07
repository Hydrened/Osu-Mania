#ifndef UTILS_H
#define UTILS_H

enum GameState {
    MAIN_MENU,
    LEVEL_PLAYING,
    LEVEL_DEAD,
    LEVEL_PAUSE,
    LEVEL_END,
};

struct Rect {
    float x, y, w, h;

    float getMinX() { return x; };
    float getMaxX() { return x + w; };
    float getMinY() { return y; };
    float getMaxY() { return y + h; };

    static bool intersect(Rect* rect1, Rect* rect2) {
        return (
            rect1->getMinX() < rect2->getMaxX() &&
            rect1->getMaxX() > rect2->getMinX() &&
            rect1->getMinY() < rect2->getMaxY() &&
            rect1->getMaxY() > rect2->getMinY()
        );
    };
};

struct Note {
    int column;
    int start;
    int end;
};

#endif