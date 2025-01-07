#include "settings.h"

// INIT
Settings::Settings(std::filesystem::path path) {
    std::ifstream file(path);
    if (file) {
        std::ostringstream ss;
        std::string line;

        while (std::getline(file, line)) ss << line << '\n';
        content = ss.str();
    } else throw std::runtime_error("HOM-201: Error reading settings file");
}

// CLEANUP
Settings::~Settings() {

}

// LOAD
std::string Settings::get(std::string setting) {
    std::string value = "";
    size_t start = content.find(setting + "=");

    if (start != std::string::npos) {
        std::string remainContent = content.substr(start + setting.length() + 1);
        
        size_t end = remainContent.find(';');
        if (end != std::string::npos) value = remainContent.substr(0, end);
    } 
    return value;
}
