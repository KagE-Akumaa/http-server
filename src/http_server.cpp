#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;
// TODO: Date 6 oct 25
// Learn about response object
// Make the response object
// connect it to the routes
struct HTTP_Request {
    string method;
    string path;
    unordered_map<string, string> headers;  // they come after the http version
    string body;
};
struct HTTP_Response {
    int statusCode;
    string statusMsg;
    unordered_map<string, string> headers;
    string body;

    void setStatus(int code, const string& msg) {
        statusCode = code;
        statusMsg = msg;
    }
    void setContentType(const string& type) { headers["Content-Type"] = type; }
    void setBody(const string& b) {
        body = b;
        headers["Content-Length"] = to_string(body.size());
    }

    // TODO: need a function to convert this all into a valid response
    string genResponse() {
        string response;

        // // Response will be based on the standard response procedure
        // string response =
        //     "HTTP/1.1 200 OK\r\n"
        //     "Content-Type: text/plain\r\n"
        //     "Content-Length: 12\r\n"
        //     "\r\n"
        //     "Hello World!";
        // // Just send the reponse

        response += "HTTP/1.1 " + to_string(statusCode) + " " + statusMsg + "\r\n";

        for (auto& [key, value] : headers) {
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
    // NOTE: This is how the strucutre of routes will look unordered_map<string, function<void(int,
    // HTTP_Request&, HTTP_Response&)>> getRoutes; request should be const no change allowed
    // unordered_map<string, function<void(int)>> getRoutes;
    unordered_map<string, function<void(int, const HTTP_Request&, HTTP_Response&)>> getRoutes;

    // NOTE: Function to handle method and path of a request
    void httpRequestParser(int clientSocket, string& method, string& path, HTTP_Request& request,
                           HTTP_Response& response) {
        char requestBuffer[8000] = {0};
        read(clientSocket, requestBuffer, sizeof(requestBuffer));

        string mainBuffer(requestBuffer);

        // TODO: need to check if headers are inside the requestbuffer if not then need to read
        // again until size become more than 32 kb or headers are done processing - DONE
        size_t headerPos = mainBuffer.find("\r\n\r\n");
        if (headerPos == string::npos) {
            // means we did not find the headers
            while (mainBuffer.size() <= 32000) {
                char tempBuffer[8000] = {0};
                headerPos = mainBuffer.find("\r\n\r\n");
                if (headerPos != string::npos) {
                    // found the headers
                    break;
                }
                // we need to read into a temp buffer and append it to main buffer
                size_t bytesRead = read(clientSocket, tempBuffer, sizeof(tempBuffer));
                if (bytesRead <= 0) {
                    // FIXME: if any errors comes in this we need to do something on the response
                    // object
                    response.setStatus(404, "Bad Request");
                    response.setContentType("text/plain");
                    string res = response.genResponse();

                    send(clientSocket, res.c_str(), res.size(), 0);
                    return;
                }
                mainBuffer.append(tempBuffer, bytesRead);
            }
        }
        int space_count = 0;
        // NOTE: now main buffer is the string which contains all the request headers
        // NOTE: Logic for parsing the path and request method
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

        string headerPart = mainBuffer.substr(0, headerPos);
        vector<string> str;
        string s = "";
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
        // Check for any string which is not pushed
        if (!s.empty()) {
            str.push_back(s);
        }
        for (string p : str) {
            string key = "", value = "";
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

        string bodyPart = mainBuffer.substr(headerPos + 4);

        size_t contentLength = 0;

        if (request.headers.count("Content-Length")) {
            const size_t maxBodyLength = 5 * 1024 * 1024;
            // means it contain body
            contentLength = stoi(request.headers["Content-Length"]);

            // Verify the size if greater than 5 mb reject
            if (contentLength > maxBodyLength) {
                // FIXME: this should be done after creating the response object reject this request
                response.setStatus(404, "Bad Request");
                response.setContentType("text/plain");
                string res = response.genResponse();

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
                        string res = response.genResponse();

                        send(clientSocket, res.c_str(), res.size(), 0);
                        return;
                    }

                    bodyPart.append(tempBuffer, bytesRead);
                }
            }

            request.body = bodyPart;
        }

        // Printing request object to verify
        cout << "PRINTING request object ---->" << endl;

        cout << request.method << " " << request.path << endl;

        for (auto it : request.headers) {
            cout << it.first << " " << it.second << endl;
        }

        cout << request.body << endl;
    }

   public:
    HTTP_SERVER(int PORT) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            cerr << "Failed to open a serverSocket" << endl;
            exit(EXIT_FAILURE);
        }
        serverSocketAddress.sin_family = AF_INET;
        serverSocketAddress.sin_port = htons(PORT);
        serverSocketAddress.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (sockaddr*)&serverSocketAddress, sizeof(serverSocketAddress)) < 0) {
            cerr << "Failed to bind the serverSocket" << endl;
            exit(EXIT_FAILURE);
        }
        if (listen(serverSocket, 10) < 0) {
            cerr << "Failed to listen!" << endl;
            exit(EXIT_FAILURE);
        } else {
            cout << "Server is Listening!!!\n";
        }
    }

    void run() {
        while (true) {
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket < 0) {
                cerr << "Client sokcet failed!" << endl;
                exit(EXIT_FAILURE);
            }

            string method, path;
            HTTP_Request request;
            HTTP_Response response;
            httpRequestParser(clientSocket, method, path, request, response);
            if (method == "GET" && getRoutes.count(path)) {
                getRoutes[path](clientSocket, request, response);
            } else {
                ifstream file("page_not_found.html");
                if (!file) {
                    cerr << "Failed to Open the page not found file\n";
                    exit(EXIT_FAILURE);
                }
                string buffer, page;

                while (getline(file, buffer)) {
                    page += buffer;
                }

                file.close();
                string response =
                    "HTTP/1.1 404 Page not found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length:" +
                    to_string(page.size()) +
                    "\r\n"
                    "\r\n" +
                    page;
                send(clientSocket, response.c_str(), response.size(), 0);
            }

            close(clientSocket);
        }
    }
    void get(const string& path, function<void(int, const HTTP_Request&, HTTP_Response&)> handler) {
        // key-> path/route value- function which will take clientsocket fd as argument
        getRoutes[path] = handler;
    }
    void stopServer() { close(serverSocket); }
};
HTTP_SERVER* globalServer = nullptr;
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
    // FIXME: we need to update the function argument to take request and response object as
    // parameters
    app.get("/", [](int clientSocket, const HTTP_Request& request, HTTP_Response& response) {
        // In this we can formulate the resoponse which we want to send to the browser

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
    app.run();
    app.stopServer();
}
