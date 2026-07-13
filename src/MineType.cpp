#include "MimeType.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <unordered_map>

std::string MimeType::get(const std::string& filename) {
    static const std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html; charset=utf-8"},
        {".htm", "text/html; charset=utf-8"},
        {".css", "text/css; charset=utf-8"},
        {".js", "application/javascript; charset=utf-8"},
        {".json", "application/json; charset=utf-8"},
        {".txt", "text/plain; charset=utf-8"},

        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},

        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".rar", "application/vnd.rar"},
        {".tar", "application/x-tar"},
        {".gz", "application/gzip"}
    };

    std::string extension =
        std::filesystem::path(filename).extension().string();

    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        }
    );

    const auto it = mimeTypes.find(extension);

    if (it != mimeTypes.end()) {
        return it->second;
    }

    return "application/octet-stream";
}