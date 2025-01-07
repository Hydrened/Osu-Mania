#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

class Settings {
private:
    std::string content;

public:
    Settings(std::filesystem::path path);
    ~Settings();

    std::string get(std::string setting);
};

#endif
