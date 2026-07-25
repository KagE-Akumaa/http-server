
#pragma once
#include "Http_Response.hpp"
#include <filesystem>
class StaticFileHandler {
  public:
    std::filesystem::path root;

    StaticFileHandler(std::filesystem::path root);

    Response HandleUserRequest(std::string &path, Response &res);
};
