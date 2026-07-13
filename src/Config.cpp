#include "Config.h"
#include "Utils.h"

#include <fstream>
#include <iostream>

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Cannot open config file: " << filename
                  << ", using default configuration.\n";
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

        const std::string key = Utils::trim(line.substr(0, equalPos));
        const std::string value = Utils::trim(line.substr(equalPos + 1));

        try {
            if (key == "PORT") {
                port_ = std::stoi(value);
            } else if (key == "ROOT_DIR") {
                rootDir_ = value;
            }
        } catch (const std::exception&) {
            std::cerr << "Invalid configuration: " << line << '\n';
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