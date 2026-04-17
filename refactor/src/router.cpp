#include "router.hpp"
// NOTE: Remember the Request method is not lowercased

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
