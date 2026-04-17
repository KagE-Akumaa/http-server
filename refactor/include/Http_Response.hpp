#pragma once

#include <string>
#include <unordered_map>
struct Response {
    // NOTE: These three are the status line
    std::string version;
    std::string message;
    int statusCode;

    // NOTE: These are the headers for Response object
    std::unordered_map<std::string, std::string> headers;

    // NOTE: This is the Response body
    std::string body;
};
