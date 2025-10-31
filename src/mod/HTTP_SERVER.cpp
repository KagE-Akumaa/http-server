#include "HTTP_SERVER.hpp"
#include <fstream>
#include <vector>

HTTP_SERVER::HTTP_SERVER(int PORT) {
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    std::cerr << "Failed to open a serverSocket" << std::endl;
    exit(EXIT_FAILURE);
  }
  serverSocketAddress.sin_family = AF_INET;
  serverSocketAddress.sin_port = htons(PORT);
  serverSocketAddress.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (sockaddr *)&serverSocketAddress,
           sizeof(serverSocketAddress)) < 0) {
    std::cerr << "Failed to bind the serverSocket" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (listen(serverSocket, 10) < 0) {
    std::cerr << "Failed to listen!" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    std::cout << "Server is Listening!!!\n";
  }
}
void HTTP_SERVER::httpRequestParser(int clientSocket, std::string &method,
                                    std::string &path, HTTP_Request &request,
                                    HTTP_Response &response) {
  char requestBuffer[8000] = {0};
  read(clientSocket, requestBuffer, sizeof(requestBuffer));

  std::string mainBuffer(requestBuffer);

  // TODO: need to check if headers are inside the requestbuffer if not then
  // need to read again until size become more than 32 kb or headers are done
  // processing - DONE
  size_t headerPos = mainBuffer.find("\r\n\r\n");
  if (headerPos == std::string::npos) {
    //
    // means we did not find the headers
    while (mainBuffer.size() <= 32000) {
      char tempBuffer[8000] = {0};
      headerPos = mainBuffer.find("\r\n\r\n");
      if (headerPos != std::string::npos) {
        // found the headers
        break;
      }
      // we need to read into a temp buffer and append it to main buffer
      size_t bytesRead = read(clientSocket, tempBuffer, sizeof(tempBuffer));
      if (bytesRead <= 0) {
        // FIXME: if any errors comes in this we need to do something on the
        // response object
        response.setStatus(404, "Bad Request");
        response.setContentType("text/plain");
        std::string res = response.genResponse();

        send(clientSocket, res.c_str(), res.size(), 0);
        return;
      }
      mainBuffer.append(tempBuffer, bytesRead);
    }
  }
  int space_count = 0;
  // NOTE: now main buffer is the std::string which contains all the request
  // headers NOTE: Logic for parsing the path and request method
  for (int i = 0; i < mainBuffer.size(); i++) {
    if (space_count == 0) {
      if (mainBuffer[i] == ' ') {
        space_count++;
        continue;
      }
      method.push_back(mainBuffer[i]);
    } else if (space_count == 1) {
      if (mainBuffer[i] == ' ') {
        space_count++;
        break;
      }
      path.push_back(mainBuffer[i]);
    } else {
      break;
    }
  }

  if (path == "/favicon.ico") {
    path = "/";
  }
  request.method = method;
  request.path = path;

  // TODO: need to make parsing logic for body and headers - DONE

  std::string headerPart = mainBuffer.substr(0, headerPos);
  std::vector<std::string> str;
  std::string s = "";
  for (int i = 0; i < headerPart.size(); i++) {
    if (headerPart[i] == '\n') {
      if (!s.empty() && s.back() == '\r') {
        s.pop_back();
      }
      str.push_back(s);
      s = "";
    } else {
      s += headerPart[i];
    }
  }
  // Check for any std::string which is not pushed
  if (!s.empty()) {
    str.push_back(s);
  }
  for (std::string p : str) {
    std::string key = "", value = "";
    bool foundColon = false;
    for (int i = 0; i < p.size(); i++) {
      if (p[i] == ':') {
        foundColon = true;
        continue;
      } else if (!foundColon) {
        key.push_back(p[i]);
      } else if (foundColon) {
        value += p[i];
      }
    }

    if (foundColon) {
      request.headers[key] = value;
    }
  }

  //        For body

  std::string bodyPart = mainBuffer.substr(headerPos + 4);

  size_t contentLength = 0;

  if (request.headers.count("Content-Length")) {
    const size_t maxBodyLength = 5 * 1024 * 1024;
    // means it contain body
    contentLength = stoi(request.headers["Content-Length"]);

    // Verify the size if greater than 5 mb reject
    if (contentLength > maxBodyLength) {
      // FIXME: this should be done after creating the response object reject
      // this request
      response.setStatus(404, "Bad Request");
      response.setContentType("text/plain");
      std::string res = response.genResponse();

      send(clientSocket, res.c_str(), res.size(), 0);
      return;
    }

    if (bodyPart.size() != contentLength) {
      //  need to read

      while (bodyPart.size() < contentLength) {
        char tempBuffer[8192] = {0};
        size_t bytesRead = read(clientSocket, tempBuffer, sizeof(tempBuffer));

        if (bytesRead <= 0) {
          // FIXME: reject request or break

          response.setStatus(404, "Bad Request");
          response.setContentType("text/plain");
          std::string res = response.genResponse();

          send(clientSocket, res.c_str(), res.size(), 0);
          return;
        }

        bodyPart.append(tempBuffer, bytesRead);
      }
    }

    request.body = bodyPart;
  }

  // Printing request object to verify
  std::cout << "PRINTING request object ---->" << std::endl;

  std::cout << request.method << " " << request.path << std::endl;

  for (auto it : request.headers) {
    std::cout << it.first << " " << it.second << std::endl;
  }

  std::cout << request.body << std::endl;
}

void HTTP_SERVER::run() {

  while (true) {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
      std::cerr << "Client sokcet failed!" << std::endl;
      exit(EXIT_FAILURE);
    }

    std::string method, path;
    HTTP_Request request;
    HTTP_Response response;
    httpRequestParser(clientSocket, method, path, request, response);
    if (method == "GET" && getRoutes.count(path)) {
      getRoutes[path](clientSocket, request, response);
    } else {
      std::ifstream file("page_not_found.html");
      if (!file) {
        std::cerr << "Failed to Open the page not found file\n";
        exit(EXIT_FAILURE);
      }
      std::string buffer, page;

      while (getline(file, buffer)) {
        page += buffer;
      }

      file.close();
      std::string response = "HTTP/1.1 404 Page not found\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length:" +
                             std::to_string(page.size()) +
                             "\r\n"
                             "\r\n" +
                             page;
      send(clientSocket, response.c_str(), response.size(), 0);
    }

    close(clientSocket);
  }
}
void HTTP_SERVER::get(
    const std::string &path,
    std::function<void(int, const HTTP_Request &, HTTP_Response &)> handler) {
  // key-> path/route value- function which will take clientsocket fd
  // as argument
  getRoutes[path] = handler;
}
void HTTP_SERVER::stopServer() { close(serverSocket); }
