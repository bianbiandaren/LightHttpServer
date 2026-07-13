#ifndef MIME_TYPE_H
#define MIME_TYPE_H

#include <string>

class MimeType {
public:
    static std::string get(const std::string& filename);
};

#endif