#include "beatmap.h"

// INIT
Beatmap::Beatmap(Game* g, std::filesystem::path d) : game(g), difficulty(d) {
    loadSong();
    loadNotes();
    delay = SDL_GetTicks();
    game->setState(LEVEL_PLAYING);
}

void Beatmap::loadSong() {
    static H2DE_Engine* engine = game->getEngine();

    std::filesystem::path dir = difficulty.parent_path();
    H2DE_LoadAssets(engine, dir);

    if (std::filesystem::exists(difficulty)) {
        std::ifstream difficultyFile(difficulty);
        std::string line;
        while (std::getline(difficultyFile, line)) {
            if (line.find("AudioFilename: ") == std::string::npos) continue;
            game->delay(game->getData()->offsets->beatmapStart, [this, line]() {
                H2DE_PlaySound(engine, 1, line.substr(15), 0);
            });
            break;
        }
    }
}

void Beatmap::loadNotes() {
    if (std::filesystem::exists(difficulty)) {
        std::ifstream file(difficulty);
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
            notes.push_back({ column, start, end });
        }
    } 
}

// CLEANUP
Beatmap::~Beatmap() {

}

// UPDATE
void Beatmap::update() {
    static int FPS = game->getFPS();
    static int offset = game->getData()->offsets->beatmapStart;

    int currentTime = SDL_GetTicks() - delay;
    if (currentTime < offset) return;

    std::vector<std::string> row = { "   ", "   ", "   ", "   " };
    for (int i = startIndexObj; i < notes.size(); i++) {
        Note note = notes[i];
        if (note.start > currentTime - offset) {
            startIndexObj = i;
            break;
        } else row[note.column - 1] = "___";
    }
    std::string strRow = "";
    for (const std::string n : row) strRow += n;
    if (strRow != "            ") std::cout << strRow << std::endl;
}

// RENDER
void Beatmap::render() {

}
