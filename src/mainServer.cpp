// Main file for the server
#include "HTTP_SERVER.hpp"
#include "json.hpp"
#include <iostream>
#include <signal.h>
using json = nlohmann::json;
using namespace std;

HTTP_SERVER *globalServer = nullptr;
void signalHandler(int signum) {
  cout << "Interupt Signal (" << signum << ")" << " recieved\n";
  if (globalServer) {
    globalServer->stopServer();
  }
  exit(signum);
}
int main() {
  const int PORT = 9000;

  signal(SIGINT, signalHandler);
  HTTP_SERVER app(PORT);
  globalServer = &app;
  // NOTE: you can do same programming as express and nodejs
  // FIXME: we need to update the function argument to take request and response
  // object as parameters
  app.get("/", [](int clientSocket, const HTTP_Request &request,
                  HTTP_Response &response) {
    // In this we can formulate the resoponse which we want to send to the
    // browser

    // // Response will be based on the standard response procedure
    // string response =
    //     "HTTP/1.1 200 OK\r\n"
    //     "Content-Type: text/plain\r\n"
    //     "Content-Length: 12\r\n"
    //     "\r\n"
    //     "Hello World!";
    // // Just send the reponse
    response.setStatus(200, "Ok");
    response.setContentType("text/plain");
    response.setBody("hello world");
    string getResponse = response.genResponse();
    cout << getResponse << endl;
    send(clientSocket, getResponse.c_str(), getResponse.size(), 0);
  });
  //    TODO: Need to make post route
  app.post("/", [](int clientSocket, const HTTP_Request &request,
                   HTTP_Response &response) {
    cout << "Printing request Body before json " << endl
         << request.body << endl;
    auto it = request.headers.find("Content-Type");
    cout << "[" << it->second << "]" << endl;

    if (it != request.headers.end() && it->second == "application/json") {
      try {
        json data = json::parse(request.body);
        string name = data["name"];
        string pass = data["pass"];

        cout << "Name " << name << " password " << pass << endl;

        response.setStatus(200, "OK");
        response.setContentType("application/json");
        response.setBody(R"({"message": "JSON received"})");
      } catch (const json::parse_error &e) {
        response.setStatus(400, "Bad Request");
        response.setContentType("application/json");
        response.setBody(R"({"error": "Invalid JSON"})");
      }
    } else {
      response.setStatus(415, "Unsupported Media Type");
      response.setBody("Expected application/json");
    }
    string getResponse = response.genResponse();
    cout << getResponse << endl;
    send(clientSocket, getResponse.c_str(), getResponse.size(), 0);
  });
  app.run();
  app.stopServer();
}
