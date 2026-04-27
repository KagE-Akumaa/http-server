
#include "StaticFileHandler.hpp"
#include <iostream>

bool insideRoot(std::filesystem::path &root,
                std::filesystem::path &resolvedPath) {
    auto rootIt = root.begin();
    auto resolvedIt = resolvedPath.begin();

    for (; rootIt != root.end(); rootIt++, resolvedIt++) {

        if (resolvedIt == resolvedPath.end()) {
            return false;
        }

        if (*rootIt != *resolvedIt) {
            return false;
        }
    }
    return true;
}
std::string stripSlashes(const std::string &userPath) {
    size_t start = userPath.find_first_not_of('/');

    if (start == std::string::npos) {
        return "";
    }

    return userPath.substr(start);
}
StaticFileHandler::StaticFileHandler(std::filesystem::path userPath) {
    // NOTE: Make this path absolute too using the canonical throws if the path
    // is not valid make sure to catch

    root = std::filesystem::canonical(userPath);
}

void printPath(const std::filesystem::path &p) {
    for (auto &it : p) {
        std::cout << it << std::endl;
    }
}
Response StaticFileHandler::HandleUserRequest(std::string &userPath,
                                              Response &res) {
    // Step - 1 To stip any leading /
    std::string path = stripSlashes(userPath);
    // Step - 2 Make this one path with root

    std::filesystem::path relativePath = path;

    // Join the 2 paths
    std::filesystem::path finalPath = root / relativePath;

    // Step - 3 Make this finalPath as absolute using weak_canonical

    std::filesystem::path resolvedPath =
        std::filesystem::weakly_canonical(finalPath);

    // Step - 4 We need to check the first N components if they are equal to
    // root the resolvedPath is valid

    printPath(root);
    printPath(resolvedPath);
    if (!insideRoot(root, resolvedPath)) {
        // return 404 response
        std::cerr << "INVALID PATH" << std::endl;
        res.status(404);
        // NOTE: We need to serve the 404 page too.
        return res;
    }

    // Step - 5 We need to check if the file exists then give the file
    if (std::filesystem::exists(resolvedPath)) {
        // We return the file
        std::cout << "File returned successfully" << std::endl;
        return res;
    }

    std::cout << "File does not exists" << std::endl;

    return res;
}
