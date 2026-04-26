
#pragma once
#include <filesystem>
class StaticFileHandler {
  public:
    std::filesystem::path root;

    StaticFileHandler(std::filesystem::path root);

    void HandleUserRequest(std::string &path);
};
