#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace Utils {

std::string trim(const std::string& text);
std::string readFile(const std::string& path);
bool isSafePath(const std::string& path);

}

#endif
