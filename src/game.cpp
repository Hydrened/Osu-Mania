#include "game.h"

// INIT
Game::Game(int argc, char** argv) {
    for (int i = 0; i < argc - 1; i++) {
        std::string arg = argv[i];
        if (arg == "-f") FPS = std::atoi(argv[i + 1]);
    }

    createWindow();
    initMainFolder();

    settings = new Settings(mainFolder / "settings.txt");
    int volume = std::stoi(settings->get("volume"));
    H2DE_SetSoundVolume(engine, -1, volume);

    loadKeys();
    openMenu();
}

void Game::createWindow() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("HOM-101: Error creating window => SDL_Init failed: " + std::string(SDL_GetError()));
    }

    window = SDL_CreateWindow("Osu! Mania - 1.0.1", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) {
        SDL_Quit();
        throw std::runtime_error("HOM-102: Error creating window => SDL_CreateWindow failed: " + std::string(SDL_GetError()));
    }
    
    int winWidth, winHeight;
    SDL_GetWindowSize(window, &winWidth, &winHeight);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("HOM-103: Error creating window => SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }

    engine = H2DE_CreateEngine(renderer, winWidth, winHeight, FPS);
    if (!engine) {
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        throw std::runtime_error("HOM-104: Error creating window => H2DE_CreateEngine failed");
    }

    SDL_SetWindowGrab(window, SDL_TRUE);
    SDL_SetWindowMinimumSize(window, winWidth, winHeight);
    SDL_SetWindowMaximumSize(window, winWidth, winHeight);
    H2DE_SetEngineMinimumSize(engine, winWidth, winHeight);
    H2DE_SetEngineMaximumSize(engine, winWidth, winHeight);
}

void Game::initMainFolder() {
    std::filesystem::path local = std::getenv("APPDATA");

    mainFolder = local / "Hydren Osu! Mania";
    if (!std::filesystem::exists(mainFolder)) {
        if (!std::filesystem::create_directory(mainFolder)) throw std::runtime_error("HOM-105: Error creating main folder");
    }

    std::filesystem::path beatmapsPath = mainFolder / "beatmaps";
    if (!std::filesystem::exists(beatmapsPath)) {
        if (!std::filesystem::create_directory(beatmapsPath)) throw std::runtime_error("HOM-106: Error creating beatmap folder");
    }

    std::filesystem::path settingsPath = mainFolder / "settings.txt";
    if (!std::filesystem::exists(settingsPath)) {
        std::ofstream settings(settingsPath);
        if (settings) {
            settings <<
                "volume=100;\n" <<
                "speed=20;\n" <<
                "key1=W;\n" <<
                "key2=X;\n" <<
                "key3=B;\n" <<
                "key4=N;\n" <<
            std::endl;
        } else throw std::runtime_error("HOM-107: Error creating settings file");
    }
}

void Game::loadKeys() {
    keys.reserve(4);
    keys.push_back(SDL_GetKeyFromName(settings->get("key1").c_str()));
    keys.push_back(SDL_GetKeyFromName(settings->get("key2").c_str()));
    keys.push_back(SDL_GetKeyFromName(settings->get("key3").c_str()));
    keys.push_back(SDL_GetKeyFromName(settings->get("key4").c_str()));
}

// CLEANUP
Game::~Game() {
    delete settings;
    if (menu) delete menu;
    if (beatmap) delete beatmap;

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    H2DE_DestroyEngine(engine);
    SDL_Quit();
}

void Game::quit() {
    isRunning = false;
}

// RUN
void Game::run() {
    Uint32 now, fpsTimer, strt, handleEventsTime, updateTime, renderTime = SDL_GetTicks();
    int frameTime;
    SDL_Event event;
    int timePerFrame = 1000 / FPS;

    while (isRunning) {
        now = SDL_GetTicks();

        handleEvents(event);
        update();
        render();
        H2DE_RenderEngine(engine);
        
        frameTime = SDL_GetTicks() - now;
        if (timePerFrame >= frameTime) SDL_Delay(timePerFrame - frameTime);
    }
}

// EVENTS
void Game::handleEvents(SDL_Event event) {
    while (SDL_PollEvent(&event)) switch (event.type) {
        case SDL_QUIT: quit(); break;

        case SDL_KEYDOWN: 
            if (state == BEATMAP_PLAYING) for (int i = 0; i < keys.size(); i++) {
                if (event.key.keysym.sym == keys[i]) beatmap->inputDown(i);
            } 
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: quit(); break;
                default: break;
            } break;
        
        case SDL_KEYUP:
            if (state == BEATMAP_PLAYING) for (int i = 0; i < keys.size(); i++) {
                if (event.key.keysym.sym == keys[i]) beatmap->inputUp(i);
            }
            break;

        case SDL_DROPFILE: switch (state) {
            case MAIN_MENU: importBeatmap(event.drop.file); break;
            default: break;
        } break;

        default: break;
    }
}

// MENU
void Game::openMenu() {
    if (menu != nullptr) return;
    if (beatmap != nullptr) closeBeatmap();
    menu = new Menu(this);
}

void Game::closeMenu() {
    if (menu) {
        delete menu;
        menu = nullptr;
    }
}

// BEATMAP
void Game::importBeatmap(char* file) {
    std::filesystem::path sourcePath = file;
    std::string fileName = sourcePath.filename().string();
    std::filesystem::path destPath = mainFolder / "songs" / fileName.substr(0, fileName.length() - 4);
    if (sourcePath.filename().extension() != ".osz") return;

    int id = -1;
    size_t pos = fileName.find(' ');
    if (pos != std::string::npos) id = std::stoi(fileName.substr(0, pos));

    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    std::string strExtractDir = destPath.string();
    const char* zipFilePath = file;
    const char* extractDir = strExtractDir.c_str();

    if (!mz_zip_reader_init_file(&zip, zipFilePath, 0)) throw std::runtime_error("HOM-202: Error opening osz file");
    if (!std::filesystem::exists(destPath)) if (!create_directory(destPath)) throw std::runtime_error("HOM-108: Error creating beatmap folder");

    int fileCount = (int)mz_zip_reader_get_num_files(&zip);
    for (int i = 0; i < fileCount; i++) {
        if (!mz_zip_reader_is_file_a_directory(&zip, i)) {
            char fileName[256];
            mz_zip_reader_get_filename(&zip, i, fileName, sizeof(fileName));

            std::string outputPath = std::string(extractDir) + "/" + fileName;
            if (!std::filesystem::exists(outputPath)) {
                if (!mz_zip_reader_extract_to_file(&zip, i, outputPath.c_str(), 0)) throw std::runtime_error("HOM-501: Error extracting file: " + std::string(fileName));
            }
        }
    }

    mz_zip_reader_end(&zip);
    SDL_free(file);
}

void Game::openBeatmap(std::filesystem::path b) {
    if (beatmap != nullptr) return;
    if (menu != nullptr) closeMenu();
    beatmap = new Beatmap(this, b);
}

void Game::closeBeatmap() { 
    if (beatmap) {
        delete beatmap;
        beatmap = nullptr;
    }
}

// UPDATE
void Game::update() {
    switch (state) {
        case BEATMAP_PLAYING: beatmap->update(); break;
        default: break;
    }
}

// RENDER
void Game::render() {
    switch (state) {
        case BEATMAP_PLAYING: beatmap->render(); break;
        default: break;
    }
}

// STATIC
void Game::delay(int ms, std::function<void()> callback) {
    std::thread([ms, callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        callback();
    }).detach();
}

// GETTER
GameData* Game::getData() const {
    return data;
}

H2DE_Engine* Game::getEngine() const {
    return engine;
}

int Game::getFPS() const {
    return FPS;
}

GameState Game::getState() const {
    return state;
}

Settings* Game::getSettings() const {
    return settings;
}

std::filesystem::path Game::getMainFolder() {
    return mainFolder;
}

// SETTER
void Game::setState(GameState s) {
    state = s;
}
