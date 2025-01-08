#include "beatmap.h"

// INIT
Beatmap::Beatmap(Game* g, std::filesystem::path b) : game(g), beatmap(b) {
    loadSettings();
    loadSong();
    loadNotes();
    delay = SDL_GetTicks();
    game->setState(BEATMAP_PLAYING);
}

void Beatmap::loadSettings() {
    static Settings* settings = game->getSettings();
    speed = std::stoi(settings->get("speed"));
}

void Beatmap::loadSong() {
    static H2DE_Engine* engine = game->getEngine();

    if (std::filesystem::exists(beatmap)) {
        std::ifstream difficultyFile(beatmap);
        std::string line;
        while (std::getline(difficultyFile, line)) {
            if (line.find("AudioFilename: ") == std::string::npos) continue;

            song = line.substr(15);
            std::filesystem::path dir = beatmap.parent_path();
            H2DE_LoadAsset(engine, dir / song);

            game->delay(game->getData()->offsets->beatmapStart, [this]() {
                H2DE_PlaySound(engine, 1, song, 0);
            });
            break;
        }
    }
}

void Beatmap::loadNotes() {
    if (std::filesystem::exists(beatmap)) {
        std::ifstream file(beatmap);
        std::string line;
        bool passedHitObjects = false;
        
        while (std::getline(file, line)) {
            if (line == "[HitObjects]") {
                passedHitObjects = true;
                continue;
            }
            if (!passedHitObjects) continue;

            std::vector<std::string> parsedLine;
            std::stringstream ss(line);
            std::string token;

            for (int i = 0; i < 6; i++) {
                std::getline(ss, token, ',');
                if (i != 0 && i != 2 && i != 5) continue;

                size_t end = token.find(':');
                if (end != std::string::npos) {
                    parsedLine.push_back(token.substr(0, end));
                } else parsedLine.push_back(token);
            }

            int column = (std::stoi(parsedLine[0]) + 64) / 128;
            int start = std::stoi(parsedLine[1]);
            int end = std::stoi(parsedLine[2]);

            Note* note = new Note();
            note->column = column;
            note->start = start;
            note->end = end;
            notes.push_back(note);
        }
    } 
}

// CLEANUP
Beatmap::~Beatmap() {
    for (Note* note : notes) delete note;
    notes.clear();
    pressedNotes.clear();
    // remove song from engine
}

// EVENTS
void Beatmap::inputDown(int key) {
    if (keysDown[key]) return;
    keysDown[key] = true;
    keyPressedThisFrame.push_back(key);
}

void Beatmap::inputUp(int key) {
    keysDown[key] = false;
    keyReleasedThisFrame.push_back(key);
}

// UPDATE
void Beatmap::update() {
    static int FPS = game->getFPS();
    static int offset = game->getData()->offsets->beatmapStart;

    // 1 => Checking for start delay
    int currentTime = SDL_GetTicks() - delay;
    if (currentTime < offset) return;

    // 2 => Checking for key pressed this frame
    for (const int key : keyPressedThisFrame) {
        Note* closestNote = getClosestNote(static_cast<int>(currentTime), key);
        if (!closestNote) continue;

        int delay = std::abs(closestNote->start - static_cast<int>(currentTime));
        Judgment judgment = getJudgment(delay);
        pressedNotes.insert(closestNote);
    }

    // 3 => Checking for key released this frame

    keyPressedThisFrame.clear();
    keyReleasedThisFrame.clear();
}

// RENDER
void Beatmap::render() {

}

// GETTER
Note* Beatmap::getClosestNote(int time, int key) {
    static int maxJudgmentTiming = game->getData()->offsets->maxJudgmentTiming;
    Note* res = nullptr;
    int closestDelta = maxJudgmentTiming;

    for (Note* note : notes) {
        if (note->start - time > maxJudgmentTiming) break;
        if (note->column != key + 1) continue;
        if (pressedNotes.find(note) != pressedNotes.end()) continue;
        
        int delta = std::abs(note->start - time);
        if (delta < closestDelta) {
            res = note;
            closestDelta = delta;
        }
    }
    return res;
}

Judgment Beatmap::getJudgment(int delay) {
    static std::map<int, Judgment> judgmentTimings = game->getData()->offsets->judgmentTimings;
    delay = std::abs(delay);

    Judgment res = MISS;
    for (const auto& [timing, judgment] : judgmentTimings) {
        if (timing == -1) continue;
        if (delay <= timing) {
            res = judgment;
            break;
        }
    }
    return res;
}
