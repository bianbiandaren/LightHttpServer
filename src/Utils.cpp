#include "Utils.h"

#include <cctype>
#include <fcntl.h>
#include <unistd.h>

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
    int fd = open(path.c_str(), O_RDONLY);

    if (fd == -1) {
        return "";
    }

    std::string content;
    char buffer[4096];

    while (true) {
        const ssize_t count = read(fd, buffer, sizeof(buffer));

        if (count > 0) {
            content.append(buffer, static_cast<std::size_t>(count));
        } else {
            break;
        }
    }

    close(fd);
    return content;
}

bool isSafePath(const std::string& path) {
    if (path.find("..") != std::string::npos) {
        return false;
    }

    if (path.find('\\') != std::string::npos) {
        return false;
    }

    return true;
}

}
