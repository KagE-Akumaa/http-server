#include "Http_Parser.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

void Parser::getRequestHeaders(std::string &requestHeader, Request &req) {
    size_t pos = 0;
    while (pos < requestHeader.size()) {
        std::string requestLine;
        size_t colonPos;
        size_t valuePos;
        std::string key;
        std::string value;

        size_t requestLinePos = requestHeader.find("\r\n", pos);
        if (requestLinePos == std::string::npos) {
            // Means last header
            requestLine = requestHeader.substr(pos);
            pos = requestHeader.size();
        } else {
            requestLine = requestHeader.substr(pos, requestLinePos - pos);
            pos = requestLinePos + 2;
        }

        colonPos = requestLine.find(':');

        if (colonPos == std::string::npos) {
            continue;
        }
        key = requestLine.substr(0, colonPos);
        valuePos = colonPos + 1;
        value = requestLine.substr(valuePos);

        trim(key);
        trim(value);

        // NOTE: This is also valid but can crash for signed char as tolower
        // expects unsigned char
        // std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        // std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        //
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c) { return tolower(c); });

        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return tolower(c); });
        req.headers[key] = value;
    }
}

void Parser::getRequestLine(std::string &requestLine, Request &req) {
    //
    std::stringstream ss(requestLine);
    std::string token;
    std::vector<std::string> tokens;

    while (ss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        std::cerr << "Tokens is empty" << std::endl;
        return;
    }

    if (tokens.size() < 3) {
        std::cerr << "Tokens is not valid" << std::endl;
        return;
    }

    req.method = tokens[0];
    req.url = tokens[1];
    req.version = tokens[2];
}
void Parser::getRequestBody(std::string &requestBody, Request &req) {
    //
}
// WARN: This function feels shaky needs testing
void Parser::trim(std::string &str) {

    if (str.empty())
        return;
    size_t start = 0, end = str.size();
    while (start < str.size() && (str[start] == ' ' || str[start] == '\t'))
        start++;
    while (end > start && (str[end] == ' ' || str[end] == '\t'))
        end--;

    if (start > end) {
        str = "";
    } else {
        str = str.substr(start, end - start + 1);
    }
}
