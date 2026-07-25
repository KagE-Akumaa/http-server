#pragma once

// NOTE: The sole job of this file is to read a file data and return it

#include <filesystem>
#include <vector>

// NOTE: Accepts the File path and return the bytes read
std::vector<char> fileReader(const std::filesystem::path &filePath);
