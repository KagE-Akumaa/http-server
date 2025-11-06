#pragma once
#include <functional>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
struct HTTP_Request {
  std::string method;
  std::string path;
  std::unordered_map<std::string, std::string>
      headers; // they come after the http version
  std::unordered_map<std::string, std::string>
      query_params; // this is for req.query_params
  std::string body;
};
struct HTTP_Response {
  int statusCode;
  std::string statusMsg;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  void setStatus(int code, const std::string &msg) {
    statusCode = code;
    statusMsg = msg;
  }
  void setContentType(const std::string &type) {
    headers["Content-Type"] = type;
  }
  void setBody(const std::string &b) {
    body = b;
    headers["Content-Length"] = std::to_string(body.size());
  }

  // TODO: need a function to convert this all into a valid response
  std::string genResponse() {
    std::string response;

    // // Response will be based on the standard response procedure
    // string response =
    //     "HTTP/1.1 200 OK\r\n"
    //     "Content-Type: text/plain\r\n"
    //     "Content-Length: 12\r\n"
    //     "\r\n"
    //     "Hello World!";
    // // Just send the reponse

    response +=
        "HTTP/1.1 " + std::to_string(statusCode) + " " + statusMsg + "\r\n";

    for (auto &[key, value] : headers) {
      response += key + ": " + value + "\r\n";
    }
    response += "\r\n";
    response += body;

    return response;
  }
};

class HTTP_SERVER {
private:
  int serverSocket;
  sockaddr_in serverSocketAddress;
  // NOTE: This is how the strucutre of routes will look unordered_map<string,
  // function<void(int, HTTP_Request&, HTTP_Response&)>> getRoutes; request
  // should be const no change allowed unordered_map<string,
  // function<void(int)>> getRoutes;
  std::unordered_map<std::string, std::function<void(int, const HTTP_Request &,
                                                     HTTP_Response &)>>
      getRoutes;
  // postRoutes
  std::unordered_map<std::string, std::function<void(int, const HTTP_Request &,
                                                     HTTP_Response &)>>
      postRoutes;

  // NOTE: Function to handle method and path of a request
  void httpRequestParser(int clientSocket, std::string &method,
                         std::string &path, HTTP_Request &request,
                         HTTP_Response &response);

public:
  HTTP_SERVER(int PORT);

  void run();
  void
  get(const std::string &path,
      std::function<void(int, const HTTP_Request &, HTTP_Response &)> handler);
  void
  post(const std::string &path,
       std::function<void(int, const HTTP_Request &, HTTP_Response &)> handler);
  void stopServer();
};
