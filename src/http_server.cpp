#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
using namespace std;
struct HTTP_Request {
    string method;
    string path;
    unordered_map<string, string> headers;  // they come after the http version
    string body;
};
class HTTP_SERVER {
   private:
    int serverSocket;
    sockaddr_in serverSocketAddress;
    unordered_map<string, function<void(int)>> getRoutes;

    // NOTE: Function to handle method and path of a request
    void httpRequestParser(int clientSocket, string& method, string& path) {
        char requestBuffer[5000] = {0};
        read(clientSocket, requestBuffer, 5000);

        cout << requestBuffer << endl;
        int space_count = 0;

        // NOTE: this is testing for HTTP_REQUEST object
        HTTP_Request request;

        // NOTE: Logic for parsing the path and request method
        for (int i = 0; requestBuffer[i] != '\0'; i++) {
            if (space_count == 0) {
                if (requestBuffer[i] == ' ') {
                    space_count++;
                    continue;
                }
                method.push_back(requestBuffer[i]);
            } else if (space_count == 1) {
                if (requestBuffer[i] == ' ') {
                    space_count++;
                    break;
                }
                path.push_back(requestBuffer[i]);
            } else {
                break;
            }
        }

        if (path == "/favicon.ico") {
            path = "/";
        }
        request.method = method;
        request.path = path;

        cout << request.method << endl << request.path << endl;
        // TODO: need to make parsing logic for body and headers

        vector<string> str;
        string s = "";
        for (int i = 0; requestBuffer[i] != '\0'; i++) {
            // if (requestBuffer[i] == '{') {
            //     break;
            // }
            if (requestBuffer[i] == '\n') {
                str.push_back(s);
                s = "";
            } else {
                s += requestBuffer[i];
            }
        }
        for (string p : str) {
            string key = "", value = "";
            bool foundColon = false;
            if (p == "\r\n\r\n") {
                break;
            }
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
        // For body 
        string findBody(requestBuffer);
        size_t pos = findBody.find("\r\n\r\n");
        if(pos != string::npos){
            string body = findBody.substr(pos + 4);

            int contentLength = 0;

            if(request.headers.find("Content-Length") != request.headers.end()){
                // in the map
                contentLength = stoi(request.headers["Content-Length"]);
            }

            if(contentLength > 0 && body.size() >= (size_t)contentLength){
                request.body = body.substr(0, contentLength);
            }
        }
        cout << "Printing the body ---->:" << endl;

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
            httpRequestParser(clientSocket, method, path);
            if (method == "GET" && getRoutes.count(path)) {
                getRoutes[path](clientSocket);
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
    void get(const string& path, function<void(int)> handler) {
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
    app.get("/", [](int clientSocket) {
        // In this we can formulate the resoponse which we want to send to the browser

        // Response will be based on the standard response procedure
        string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Hello World!";
        // Just send the reponse
        send(clientSocket, response.c_str(), response.size(), 0);
    });
    app.run();
    app.stopServer();
}
