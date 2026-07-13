#include "Utils.h"

#include <cctype>
#include <fstream>
#include <iterator>

namespace Utils {

std::string trim(const std::string& value) {
    std::size_t begin = 0;

    while (
        begin < value.size() &&
        std::isspace(static_cast<unsigned char>(value[begin]))
    ) {
        ++begin;
    }

    std::size_t end = value.size();

    while (
        end > begin &&
        std::isspace(static_cast<unsigned char>(value[end - 1]))
    ) {
        --end;
    }

    return value.substr(begin, end - begin);
}

bool readFile(
    const std::string& filename,
    std::string& content
) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    content.assign(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );

    return true;
}

bool isSafePath(const std::string& path) {
    return path.find("..") == std::string::npos;
}

}