#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
    bool load(const std::string& filename);

    int port() const;
    const std::string& rootDir() const;

private:
    int port_ = 8080;
    std::string rootDir_ = "www";
};

#endif