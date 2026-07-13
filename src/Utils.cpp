#include "Utils.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace Utils {

std::string trim(const std::string& text) {
    std::size_t begin = 0;

    while (
        begin < text.size() &&
        std::isspace(static_cast<unsigned char>(text[begin]))
    ) {
        ++begin;
    }

    std::size_t end = text.size();

    while (
        end > begin &&
        std::isspace(static_cast<unsigned char>(text[end - 1]))
    ) {
        --end;
    }

    return text.substr(begin, end - begin);
}

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

bool isSafePath(const std::string& path) {
    return path.find("..") == std::string::npos;
}

}
