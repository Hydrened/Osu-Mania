#ifndef DATA_H
#define DATA_H

#include <map>

struct GameData {
    struct Positions {

    };

    struct Sizes {

    };

    struct Offsets {
        int beatmapStart = 3000;
        std::map<int, Judgment> judgmentTimings;
        int maxJudgmentTiming = 200;

        Offsets() {
            judgmentTimings[-1] = MISS;
            judgmentTimings[16] = MARVELLOUS;
            judgmentTimings[43] = PERFECT;
            judgmentTimings[97] = GREAT;
            judgmentTimings[133] = GOOD;
            judgmentTimings[maxJudgmentTiming] = OK;
        }
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
