#include "MimeResolver.hpp"
#include <algorithm>
#include <iostream>

// NOTE: Define static member - Right now support this much :)
const std::unordered_map<std::string, std::string> MimeResolver::mimeMap = {
    {"html", "text/html"},
    {"css", "text/css"},
    {"js", "application/javascript"},
    {"json", "application/json"},
    {"png", "image/png"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"gif", "image/gif"},
    {"svg", "image/svg+xml"},
    {"txt", "text/plain"}};

// NOTE: This functon will return the mime type for a filePath

std::string
MimeResolver::getMimeType(const std::filesystem::path &filePath) const {

    std::cout << filePath << std::endl;

    // WE Need to get the .png so

    std::string path = filePath;

    size_t pos = path.rfind('.');

    if (pos == std::string::npos) {
        return "";
    }

    // 2. Got the extension
    std::string ext = path.substr(pos + 1);

    // 3. Normalize it make it lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // 4. Match the mime type
    auto it = mimeMap.find(ext);
    if (it != mimeMap.end()) {
        return it->second;
    }

    // 5. Not match return fallback
    return "application/octet-stream";
}
