#include "Config.h"
#include "Utils.h"

#include <fstream>

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        line = Utils::trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        const std::size_t equalPos = line.find('=');

        if (equalPos == std::string::npos) {
            continue;
        }

        const std::string key =
            Utils::trim(line.substr(0, equalPos));

        const std::string value =
            Utils::trim(line.substr(equalPos + 1));

        if (key == "PORT") {
            if (!value.empty()) {
                port_ = std::stoi(value);
            }
        } else if (key == "ROOT_DIR") {
            if (!value.empty()) {
                rootDir_ = value;
            }
        }
    }

    return true;
}

int Config::port() const {
    return port_;
}

const std::string& Config::rootDir() const {
    return rootDir_;
}
