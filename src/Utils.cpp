#include "Utils.h"

#include <cctype>

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

}