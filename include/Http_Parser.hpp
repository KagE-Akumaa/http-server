
#pragma once
#include "Http_Request.hpp"
class Parser {
  private:
    void trim(std::string &str);

  public:
    void getRequestLine(std::string &requestLine, Request &req);
    void getRequestHeaders(std::string &requestHeader, Request &req);
    void getRequestBody(std::string &requestBody, Request &req);
};
