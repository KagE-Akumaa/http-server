#include "Http_Response.hpp"

void Response::status(int statusCode) { this->statusCode = statusCode; }

void Response::setBody(const std::string &body) {
    this->body = body;
    // NOTE: Headers are stored in lowercase for case-insensitive lookup.
    //  The serialization layer is responsible for converting them
    //  to canonical HTTP format when sending the response.
    headers["content-length"] = std::to_string(body.size());
}

void Response::setContentType(const std::string &type) {
    headers["content-type"] = type;
}

int Response::getStatusCode() { return statusCode; }

void Response::send() {
    // This will call the serialization method
}
