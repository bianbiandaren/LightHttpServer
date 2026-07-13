#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace Utils {

std::string trim(const std::string& value);

bool readFile(
    const std::string& filename,
    std::string& content
);

bool isSafePath(const std::string& path);

}

#endif