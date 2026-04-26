#include "StaticFileHandler.hpp"
#include <algorithm>
#include <iostream>

StaticFileHandler::StaticFileHandler(std::filesystem::path userPath) {
    // NOTE: Make this path absolute too using the canonical throws if the path
    // is not valid make sure to catch

    root = std::filesystem::canonical(userPath);
}

void StaticFileHandler::HandleUserRequest(std::string &path) {
    // Step - 1 To stip any leading /

    std::reverse(path.begin(), path.end());

    while (!path.empty() && path.back() == '/') {
        path.pop_back();
    }
    if (path.empty()) {
        std::cerr << "Invalid request Path" << std::endl;
        return;
    }

    std::reverse(path.begin(), path.end());

    // Step - 2 Make this one path with root

    std::filesystem::path relativePath = path;

    // Join the 2 paths
    std::filesystem::path finalPath = root / relativePath;

    // Step - 3 Make this finalPath as absolute using weak_canonical

    std::filesystem::path resolvedPath =
        std::filesystem::weakly_canonical(finalPath);

    // Step - 4 We need to check the first N components if they are equal to
    // root the resolvedPath is valid

    for (auto &p : root) {
        if (p != resolvedPath) {
            // Invalid
        }
    }
}
