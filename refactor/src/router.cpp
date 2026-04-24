#include "router.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
// NOTE: Remember the Request method is not lowercased
void fallback(const Request &req, Response &res) {
    std::ifstream file("assets/404.html");
    if (!file) {
        std::cerr << "Failed to open the file" << std::endl;
        return;
    }
    std::string buffer, page;

    while (std::getline(file, buffer)) {
        page += buffer;
    }

    file.close();
    res.status(404);
    res.setContentType("text/html");
    res.setBody(page);
}
std::vector<char> readFile(const std::string &path) {
    // 1. Open file in binary mode
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open the file" << std::endl;
        return {};
    }

    // 2. Move to end to get size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();

    // 3. Move back to beginning
    file.seekg(0, std::ios::beg);

    // 4. Allocate buffer
    std::vector<char> buffer(size);

    // 5. Read file into buffer
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Failed to open the file" << std::endl;
        return {};
    }

    return buffer;
}
void staticFileHandler(const Request &req, Response &res) {
    //
    // Step 1 - Check if file is there. if not found return the 404 page
    std::ifstream file("." + req.url);
    if (!file) {
        std::cout << "File not found" << std::endl;
        fallback(req, res);
        return;
    }

    // FIX: Assumption - There are no query parameters
    //  Step 2 - Parse the type of file We assume that there are no query
    //  parameters right now

    std::string temp = "." + req.url;
    std::string fileType;

    std::cout << req.url << std::endl;
    for (int i = temp.size() - 1; i >= 0; i--) {
        if (temp[i] == '.') {
            break;
        }
        fileType += temp[i];
    }
    std::reverse(fileType.begin(), fileType.end());
    std::cout << fileType << std::endl;
    if (fileType == "png" || fileType == "jpg") {
        res.setContentType("image/" + fileType);
    }

    auto data = readFile(temp);
    file.close();
    res.status(200);
    res.setBody(data);
}
// NOTE: This function -> matches the correct route for the routehandler
void Router::match(const Request &req, Response &res) {
    // TODO: Response are not valid
    std::string path = req.url;

    auto dispatcher =
        [&](std::function<void(const Request &, Response &)> &fn) {
            fn(req, res);
        };

    if (req.method == "GET") {
        if (getRoutes.find(path) != getRoutes.end()) {
            // Means we found the match
            dispatcher(getRoutes[path]);
        } else {
            // NOTE: Two cases - If the route path is a file path we give that
            // file else we just give the 404 page

            staticFileHandler(req, res);
        }
        // FIX: return after giving a valid Response
        // return;
    } else if (req.method == "POST") {
        if (postRoutes.find(path) != postRoutes.end()) {
            // Means we found the match
            dispatcher(postRoutes[path]);
        }
        // FIX: return after giving a valid Response
        // return;
        else {
            std::ifstream file("assets/404.html");
            if (!file) {
                std::cerr << "Failed to open the file" << std::endl;
                return;
            }
            std::string buffer, page;

            while (std::getline(file, buffer)) {
                page += buffer;
            }

            file.close();
            res.status(404);
            res.setContentType("text/html");
            res.setBody(page);
        }
    }
}

// NOTE: Add the path to a designated get routehandler
void Router::get(const std::string &path, RouteHandler rt) {
    getRoutes[path] = rt;
}

// NOTE: Add the path to a designated post routehandler
void Router::post(const std::string &path, RouteHandler rt) {
    postRoutes[path] = rt;
}

// NOTE: This add the Middleware to the vector
void Router::use(const Middleware mi) { middle.push_back(mi); }
