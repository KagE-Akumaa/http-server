#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
class MimeResolver {
  private:
    // NOTE: Made this static and const need to initialize once and don't need
    // to modify it
    static const std::unordered_map<std::string, std::string> mimeMap;

  public:
    std::string getMimeType(const std::filesystem::path &filePath) const;
};
