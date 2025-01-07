#ifndef DATA_H
#define DATA_H

struct GameData {
    struct Positions {

    };

    struct Sizes {

    };

    struct Offsets {
        int beatmapStart = 3000;
    };

    ~GameData() {
        delete positions;
        delete sizes;
        delete offsets;
    };

    Positions* positions = new Positions();
    Sizes* sizes = new Sizes();
    Offsets* offsets = new Offsets();
};

#endif
