#ifndef DATA_H
#define DATA_H

#include <map>
#define BLOCKS_ON_WIDTH 22.0f

struct GameData {
    struct Positions {
        BeatmapPos noteBg = { 0.0f, -2.04f };
        BeatmapPos noteBgBorder = { -0.1f, -2.04f };
        BeatmapPos key = { 0.0f, -2.04f };
        BeatmapPos keyBar = { 0.0f, -0.27f };
        BeatmapPos healthBar = { 4.0f, -2.54f };
        BeatmapPos judgment = { 1.0f, 1.3f };
    };

    struct Sizes {
        BeatmapSize noteBg = { 4.0f, 20.0f };
        BeatmapSize noteBgBorder = { 4.2f, 20.0f };
        BeatmapSize key = { 1.0f, 1.72f };
        BeatmapSize keyBar = { 4.0f, 0.57f };
        BeatmapSize healthBar = { 7.81f, 0.5f };

        BeatmapSize note = { 1.0f, 0.23f };

        std::unordered_map<Judgment, BeatmapSize> judgments;

        Sizes() {
            judgments[MISS] = { 2.0f, 1.0f };
            judgments[MARVELLOUS] = { 2.0f, 1.0f };
            judgments[PERFECT] = { 2.0f, 1.0f };
            judgments[GREAT] = { 2.0f, 1.0f };
            judgments[GOOD] = { 2.0f, 1.0f };
            judgments[OK] = { 2.0f, 1.0f };
        }
    };

    struct Offsets {
        int beatmapStart = 3000;
        int maxJudgmentTiming = 200;

        float hitZone = 2.04f;

        std::map<int, Judgment> judgmentTimings;

        Offsets() {
            judgmentTimings[-1] = MISS;
            judgmentTimings[16] = MARVELLOUS;
            judgmentTimings[43] = PERFECT;
            judgmentTimings[97] = GREAT;
            judgmentTimings[133] = GOOD;
            judgmentTimings[200] = OK;
        }
    };

    struct Others {
        float maxPixelPerMsSpeed = 3.0330f;
        float pixelLostPerSpeed = 0.07207f;

        std::unordered_map<Judgment, std::string> stringifiedJudgment;

        Others() {
            stringifiedJudgment[MISS] = "miss";
            stringifiedJudgment[MARVELLOUS] = "marvellous";
            stringifiedJudgment[PERFECT] = "perfect";
            stringifiedJudgment[GREAT] = "great";
            stringifiedJudgment[GOOD] = "good";
            stringifiedJudgment[OK] = "ok";
        }
    };

    ~GameData() {
        delete positions;
        delete sizes;
        delete offsets;
        delete others;
    };

    Positions* positions = new Positions();
    Sizes* sizes = new Sizes();
    Offsets* offsets = new Offsets();
    Others* others = new Others();
};

#endif
