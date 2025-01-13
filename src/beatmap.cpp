#include "beatmap.h"

// INIT
Beatmap::Beatmap(Game* g, std::filesystem::path b) : game(g), beatmap(b) {
    loadSettings();
    loadSong();
    loadBgPicture();
    loadNotes();
    
    accDelay = SDL_GetTicks();
    game->setState(BEATMAP_PLAYING);
}

void Beatmap::loadSettings() {
    static Settings* settings = game->getSettings();
    speed = std::stoi(settings->get("speed"));
    bgOpacity = static_cast<Uint8>(255 * (std::stoi(settings->get("bg_opacity")) / 100.0f));
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

void Beatmap::loadBgPicture() {
    static H2DE_Engine* engine = game->getEngine();

    if (std::filesystem::exists(beatmap)) {
        std::ifstream difficultyFile(beatmap);
        std::string line;
        while (std::getline(difficultyFile, line)) {
            size_t index = (line.find(".jpg") != std::string::npos) ? line.find(".jpg") : line.find(".png");
            if (index == std::string::npos) continue;

            bgPicture = line.substr(5, index - 5 + 4);
            std::filesystem::path dir = beatmap.parent_path();
            H2DE_LoadAsset(engine, dir / bgPicture);
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
    slidersHolding.clear();
    sliderJudgments.clear();
    judgments.clear();

    H2DE_RemoveAsset(game->getEngine(), song);
    H2DE_RemoveAsset(game->getEngine(), bgPicture);
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

void Beatmap::addJudgment(Judgment judgment) {
    static std::unordered_map<Judgment, std::string> strJudgments = {
        { MARVELLOUS, "300+" },
        { PERFECT, "300" },
        { GREAT, "200" },
        { GOOD, "100" },
        { OK, "50" },
        { MISS, "MISS" },
    };
    judgments[judgment]++;
    thisFrameJudgment = new Judgment(judgment);
    currentJudgment = new Judgment(judgment);
}

// UPDATE
void Beatmap::update() {
    static int offset = game->getData()->offsets->beatmapStart;
    static int maxJudgmentTiming = game->getData()->offsets->maxJudgmentTiming;

    H2DE_TickTimelineManager(tm);

    // 1 => Checking for start delay
    int currentTime = SDL_GetTicks() - accDelay - offset;
    if (currentTime < 0) return;

    // 2 => Checking for key pressed this frame
    for (const int key : keyPressedThisFrame) {
        Note* closestNote = getClosestNote(static_cast<int>(currentTime), key);
        if (!closestNote) continue;

        int delay = std::abs(closestNote->start - static_cast<int>(currentTime));
        Judgment judgment = getJudgment(delay);
        addJudgment(judgment);
        pressedNotes.insert(closestNote);

        if (closestNote->end != 0) {
            slidersHolding.push_back(closestNote);
            sliderJudgments[closestNote] = judgment;
        }
    }

    // 3 => Checking for key released this frame
    for (const int key : keyReleasedThisFrame) {
        for (int i = 0; i < slidersHolding.size(); i++) {
            Note* note = slidersHolding[i];

            if (note->column == key + 1) {
                int delay = note->end - static_cast<int>(currentTime);
                Judgment judgment = (delay < -maxJudgmentTiming) ? static_cast<Judgment>(sliderJudgments[note] + 1) : getJudgment(std::abs(delay));
                if (judgment == MISS) judgment = OK;
                addJudgment(judgment);

                slidersHolding.erase(slidersHolding.begin() + i);
                break;
            }
        }
    }

    // 4 => Checking for misses
    for (Note* note : notes) {
        if (pressedNotes.find(note) == pressedNotes.end()) {
            int delay = note->start - static_cast<int>(currentTime);
            if (delay < -maxJudgmentTiming) {
                addJudgment(MISS);
                pressedNotes.insert(note);
            } else if (delay > maxJudgmentTiming) break;
        }
    }

    // 5 => Reset
    keyPressedThisFrame.clear();
    keyReleasedThisFrame.clear();
}

// RENDER
void Beatmap::render() {
    renderBackground();
    renderColumn();
    renderNotes();
    // renderJudgment();
}

void Beatmap::renderBackground() {
    static H2DE_Engine* engine = game->getEngine();
    int winWidth, winHeight;
    game->getWinSize(&winWidth, &winHeight);

    H2DE_GraphicObject* picture = H2DE_CreateGraphicObject();
    picture->type = IMAGE;
    picture->pos = { 0, 0 };
    picture->size = { winWidth, winHeight };
    picture->texture = bgPicture;
    picture->rgb = { bgOpacity, bgOpacity, bgOpacity, 255 };
    picture->index = 1;
    H2DE_AddGraphicObject(engine, picture);
}

void Beatmap::renderColumn() {
    static H2DE_Engine* engine = game->getEngine();
    static Calculator* calculator = game->getCalculator();
    static GameData* gameData = game->getData();



    // 1 => Render black background
    static H2DE_Pos absNoteBgPos = calculator->convertToPx(gameData->positions->noteBg, gameData->sizes->noteBg);
    static H2DE_Size absNoteBgSize = calculator->convertToPx(gameData->sizes->noteBg);

    H2DE_GraphicObject* noteBg = H2DE_CreateGraphicObject();
    noteBg->type = POLYGON;
    noteBg->pos = absNoteBgPos;
    noteBg->points = {
        { 0, 0 },
        { absNoteBgSize.w, 0 },
        { absNoteBgSize.w, absNoteBgSize.h },
        { 0, absNoteBgSize.h },
    };
    noteBg->rgb = { 0, 0, 0, 255 };
    noteBg->filled = true;
    noteBg->index = 4;
    H2DE_AddGraphicObject(engine, noteBg);



    // 2 => Render borders
    static H2DE_Pos absNoteBgBorderPos = calculator->convertToPx(gameData->positions->noteBgBorder, gameData->sizes->noteBgBorder);
    static H2DE_Size absNoteBgBorderSize = calculator->convertToPx(gameData->sizes->noteBgBorder);

    H2DE_GraphicObject* noteBgBorder = H2DE_CreateGraphicObject();
    noteBgBorder->type = POLYGON;
    noteBgBorder->pos = absNoteBgBorderPos;
    noteBgBorder->points = {
        { 0, 0 },
        { absNoteBgBorderSize.w, 0 },
        { absNoteBgBorderSize.w, absNoteBgBorderSize.h },
        { 0, absNoteBgBorderSize.h },
    };
    noteBgBorder->rgb = { 127, 127, 127, 255 };
    noteBgBorder->filled = true;
    noteBgBorder->index = 3;
    H2DE_AddGraphicObject(engine, noteBgBorder);



    // 3 => Render 4 bottom keys
    static BeatmapPos keyPos = gameData->positions->key;
    static BeatmapSize keySize = gameData->sizes->key;
    static H2DE_Size absKeySize = calculator->convertToPx(keySize);

    for (int i = 0; i < 4; i++) {
        H2DE_Pos absPos = calculator->convertToPx({ i * 1.0f, keyPos.y }, keySize);
        std::string keyID = (i == 0 || i == 3) ? "1" : "2";
        std::string keyState = (keysDown[i]) ? "D" : "";

        H2DE_GraphicObject* key = H2DE_CreateGraphicObject();
        key->type = IMAGE;
        key->pos = absPos;
        key->size = absKeySize;
        key->texture = "mania-key" + keyID + keyState + ".png";
        key->index = 10;
        H2DE_AddGraphicObject(engine, key);
    }



    // 4 => Render key top bar
    static H2DE_Pos absKeyBarPos = calculator->convertToPx(gameData->positions->keyBar, gameData->sizes->keyBar);
    static H2DE_Size absKeyBarSize = calculator->convertToPx(gameData->sizes->keyBar);

    H2DE_GraphicObject* keyBar = H2DE_CreateGraphicObject();
    keyBar->type = IMAGE;
    keyBar->pos = absKeyBarPos;
    keyBar->size = absKeyBarSize;
    keyBar->texture = "mania-stage-hint.png";
    keyBar->index = 11;
    H2DE_AddGraphicObject(engine, keyBar);



    // 5 => Render health bar
    static H2DE_Pos absHealthBarPos = calculator->convertToPx(gameData->positions->healthBar, gameData->sizes->healthBar);
    static H2DE_Size absHealthBarSize = calculator->convertToPx(gameData->sizes->healthBar);

    H2DE_GraphicObject* healthBar = H2DE_CreateGraphicObject();
    healthBar->type = IMAGE;
    healthBar->pos = absHealthBarPos;
    healthBar->size = absHealthBarSize;
    healthBar->texture = "scorebar-bg@2x.png";
    healthBar->rotation = 270;
    healthBar->index = 15;
    H2DE_AddGraphicObject(engine, healthBar);
}

void Beatmap::renderNotes() {
    static H2DE_Engine* engine = game->getEngine();
    static Calculator* calculator = game->getCalculator();
    static GameData* gameData = game->getData();
    static int offset = gameData->offsets->beatmapStart;
    static float pixelLostPerSpeed = gameData->others->pixelLostPerSpeed;
    static H2DE_Size absNoteSize = calculator->convertToPx(gameData->sizes->note);
    static float hitZone = gameData->offsets->hitZone;

    int winWidth, winHeight;
    game->getWinSize(&winWidth, &winHeight);
    static int blockSize = static_cast<int>(winWidth / BLOCKS_ON_WIDTH);

    float pxPerMs = gameData->others->maxPixelPerMsSpeed - (40 - speed) * pixelLostPerSpeed;
    int currentTime = SDL_GetTicks() - accDelay - offset;

    for (Note* note : notes) {
        if (pressedNotes.find(note) != pressedNotes.end()) continue;
        
        int delay = note->start - currentTime;
        int yAbsPos = (winHeight - delay * pxPerMs) - blockSize * hitZone;

        if (yAbsPos >= winHeight) break;

        std::string columnID = (note->column == 1 || note->column == 4) ? "1" : "2";
        H2DE_Pos absNotePos = calculator->convertToPx({ note->column - 1.0f, 0.0f }, gameData->sizes->note);

        H2DE_GraphicObject* graphicNote = H2DE_CreateGraphicObject();
        graphicNote->type = IMAGE;
        graphicNote->pos = { absNotePos.x, yAbsPos };
        graphicNote->size = absNoteSize;
        graphicNote->texture = "mania-note" + columnID + ".png";
        graphicNote->index = 5;
        H2DE_AddGraphicObject(engine, graphicNote);
    }
}

void Beatmap::renderJudgment() {
    static H2DE_Engine* engine = game->getEngine();
    static Calculator* calculator = game->getCalculator();
    static GameData* gameData = game->getData();

    if (!currentJudgment) return;

    BeatmapSize judgmentSize = gameData->sizes->judgments[*currentJudgment];
    H2DE_Pos absJudgmentPos = calculator->convertToPx(gameData->positions->judgment, judgmentSize);
    H2DE_Size absJudgmentSize = calculator->convertToPx(judgmentSize);

    H2DE_GraphicObject* judgment = H2DE_CreateGraphicObject();
    judgment->type = IMAGE;
    judgment->pos = absJudgmentPos;
    judgment->size = absJudgmentSize;
    judgment->texture = gameData->others->stringifiedJudgment[*currentJudgment] + ".png";
    judgment->scaleOrigin = { static_cast<int>(absJudgmentSize.w / 2), static_cast<int>(absJudgmentSize.h / 2) };
    judgment->scale = { currentJudgmentScale, currentJudgmentScale };
    judgment->index = 20;
    H2DE_AddGraphicObject(engine, judgment);

    if (!thisFrameJudgment) return;

    H2DE_AddTimelineToManager(tm, H2DE_CreateTimeline(engine, 50, EASE_OUT, [this](float blend) {
        currentJudgmentScale = 1.0f + blend * (1.5f - 1.0f);
    }, [this]() {
        delete currentJudgment;
        currentJudgment = nullptr;
    }, 0));

    delete thisFrameJudgment;
    thisFrameJudgment = nullptr;
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
