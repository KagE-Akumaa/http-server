#pragma once
// QUES: What is the responsibility of router?
// ANS: To match the route with correct route handler.
#include "Http_Request.hpp"
#include "Http_Response.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
using nextFnc = std::function<void()>;
using RouteHandler = std::function<void(const Request &, Response &)>;
using Middleware = std::function<void(const Request &, Response &, nextFnc)>;
class Router {
  private:
    std::vector<Middleware> middle;
    std::unordered_map<std::string, RouteHandler> getRoutes;
    std::unordered_map<std::string, RouteHandler> postRoutes;

  public:
    void get(const std::string &path, RouteHandler);
    void post(const std::string &path, RouteHandler);
    void use(const Middleware);
    void match(const Request &, Response &);
};
