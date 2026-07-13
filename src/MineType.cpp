#include "MimeType.h"

#include <map>
#include <string>

std::string MimeType::get(const std::string& filename) {
    static std::map<std::string, std::string> mimeTypes = {
        {".html", "text/html; charset=utf-8"},
        {".htm", "text/html; charset=utf-8"},
        {".css", "text/css; charset=utf-8"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".txt", "text/plain; charset=utf-8"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".rar", "application/x-rar-compressed"},
        {".tar", "application/x-tar"},
        {".gz", "application/gzip"}
    };

    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream";
    }

    std::string extension = filename.substr(dotPos);
    auto it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) {
        return it->second;
    }

    return "application/octet-stream";
}