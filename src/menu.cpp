#include "menu.h"

// INIT
Menu::Menu(Game* g) : game(g) {
    loadBeatmaps();
    openBeatmap(2102159, "Insane");
}

void Menu::loadBeatmaps() {
    for (const std::filesystem::path beatmap : std::filesystem::directory_iterator(game->getMainFolder() / "beatmaps")) {
        std::string beatmapName = beatmap.filename().string();
        size_t spaceIndex = beatmapName.find(' ');
        if (spaceIndex != std::string::npos) {
            int id = -1;
            try {
                id = std::stoi(beatmapName.substr(0, spaceIndex));
            }
            catch(const std::exception& e) {
                std::cerr << "Beatmap: " << '"' << beatmapName << '"' << " does not have a valid id: " << e.what() << std::endl;
                return;
            }
            if (id == -1) continue;

            for (const std::filesystem::path file : std::filesystem::directory_iterator(beatmap)) {
                if (file.extension() != ".osu") continue;

                std::string strFile = file.string();                    
                size_t opSqBrIndex = strFile.find('[');
                size_t enSqBrIndex = strFile.find(']');

                if (opSqBrIndex != std::string::npos && enSqBrIndex != std::string::npos) {
                    std::string difficulty = strFile.substr(opSqBrIndex + 1, enSqBrIndex - opSqBrIndex - 1);
                    beatmaps[id][difficulty] = file;
                }
            }
        }
    }
}

// CLEANUP
Menu::~Menu() {

}

// EVENTS
void Menu::openBeatmap(int id, std::string difficulty) {
    if (beatmaps.find(id) == beatmaps.end()) {
        std::cerr << "Beatmap id " << '"' << id << '"' << " is not valid" << std::endl;
        return;
    }
    if (beatmaps[id].find(difficulty) == beatmaps[id].end()) {
        std::cerr << "Beatmap difficulty " << '"' << difficulty << '"' << " does not exist for beatmap id: " << id << std::endl;
        return;
    }
    game->openBeatmap(beatmaps[id][difficulty]);
}
