#pragma once
#include <string>
#include <unordered_map>
struct Request {
    // NOTE: These three are the request line the first line of the request
    // object
    std::string method;
    std::string url;
    std::string version;
    // NOTE: These are the header lines up until \r\n\r\n
    std::unordered_map<std::string, std::string> headers;
    // NOTE: This is the request body if the header lines contains
    // content-length
    std::string body;
};
