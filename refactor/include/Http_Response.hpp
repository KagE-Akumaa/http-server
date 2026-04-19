#pragma once

#include <string>
#include <unordered_map>
class Response {
    // NOTE: These three are the status line
    std::string version;
    std::string message;
    int statusCode;

    // NOTE: These are the headers for Response object
    std::unordered_map<std::string, std::string> headers;

    // NOTE: This is the Response body
    std::string body;

    void send();

  public:
    // NOTE: This will set the statusCode, message will be set by the
    // serialization layer at server level this will just fill it
    void status(int statusCode);

    void setBody(const std::string &body);

    void setContentType(const std::string &type);

    int getStatusCode();

    friend class HTTP_SERVER;
};
