#include "Http_Server.hpp"
#include "Http_Parser.hpp"
#include "Http_Request.hpp"
#include "router.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
// FIX: Delete this later :)
void debug(Request &req) {
        std::cout << "Logging request Line ---> " << std::endl;
        std::cout << req.method << " " << req.url << " " << req.version
                  << std::endl;
        std::cout << "Logging request Headers ---> " << std::endl;

        for (auto &it : req.headers) {
                std::cout << "Key: " << it.first << " "
                          << "Value: " << it.second << std::endl;
        }

        std::cout << "Logging request Body ---> " << std::endl;

        std::cout << req.body << std::endl;
}

namespace {
bool shouldKeepAlive(const Request &req) {
        auto it = req.headers.find("connection");
        const std::string connection =
            (it != req.headers.end()) ? it->second : "";

        if (connection.find("close") != std::string::npos) {
                return false;
        }

        if (req.version == "HTTP/1.1") {
                return true;
        }

        if (req.version == "HTTP/1.0") {
                return connection.find("keep-alive") != std::string::npos;
        }

        return false;
}

bool sendAll(int fd, const std::string &data) {
        size_t sent = 0;
        while (sent < data.size()) {
                ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
                if (n <= 0) {
                        return false;
                }
                sent += static_cast<size_t>(n);
        }
        return true;
}
} // namespace

HTTP_SERVER::HTTP_SERVER(int PORT, Router &r) : r(r) {

        // Get the socket fd we chose the tcp socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
                throw std::runtime_error(
                    std::string("Failed to create a socket: ") +
                    std::strerror(errno));
        }
        // NOTE: I have had this problem where if i close the server it
        // won't bind instantaly it needs some time , now i know why it
        // exists it's the os which tells it to wait for about 60s and then
        // the port will be available for use next time - The fix to this is
        // use the setsockopt() (set socket option) to set the option to
        // SO_REUSEADDR
        serverSocketAddress.sin_family = AF_INET;
        serverSocketAddress.sin_port = htons(PORT);
        serverSocketAddress.sin_addr.s_addr = INADDR_ANY;

        int on = 1; // Non-zero value is Used to enable the SO_REUSEADDR
                    // (socket reuse address)
        // NOTE: This means that set the current socket to reuse immediately
        // after closing
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &on,
                       sizeof(on)) < 0) {
                throw std::runtime_error(
                    std::string("Failed to add socket option: ") +
                    std::strerror(errno));
        }

        // Now we need to bind the socket to the port
        if (bind(serverSocket, (sockaddr *)&serverSocketAddress,
                 sizeof(serverSocketAddress)) < 0) {
                throw std::runtime_error(
                    std::string("Failed to bind the socket: ") +
                    std::strerror(errno));
        }

        // Now we listen on port for any incomming requests we can add the
        // size of the queue too
        if (listen(serverSocket, 10) < 0) {
                throw std::runtime_error(std::string("Failed to listen: ") +
                                         std::strerror(errno));
        }
        // TODO: we can make a loggin function too instead of this just pass
        // the string and done
        std::cout << "Server is listening" << std::endl;
}
HTTP_SERVER::~HTTP_SERVER() {
        if (serverSocket != -1)
                close(serverSocket);
}

void HTTP_SERVER::connectionHandler(ClientSocket clientFd) {
        // NOTE: Keep the TCP connection open across requests. Extra bytes
        // already read (pipelined requests) stay in leftover for the next
        // cycle. The connection closes when the client sends Connection:
        // close, uses HTTP/1.0 without keep-alive, or a read/write fails.
        std::string leftover;
        std::vector<char> requestBuffer(8000, 0);

        while (true) {
                std::string request = std::move(leftover);
                leftover.clear();

                while (request.find("\r\n\r\n") == std::string::npos) {
                        ssize_t bytesRead =
                            read(clientFd.getFd(), requestBuffer.data(),
                                 requestBuffer.size());

                        if (bytesRead == 0) {
                                return;
                        }
                        if (bytesRead == -1) {
                                std::cerr << "Failed reading the http-request: "
                                          << std::strerror(errno);
                                return;
                        }

                        request.append(requestBuffer.begin(),
                                       requestBuffer.begin() + bytesRead);
                }

                Request req;
                Response res;
                Parser p;

                size_t requestLinePos = request.find("\r\n");
                if (requestLinePos == std::string::npos) {
                        return;
                }

                std::string requestLine = request.substr(0, requestLinePos);

                p.getRequestLine(requestLine, req);

                size_t requestHeaderStart = requestLinePos + 2;
                size_t requestHeaderPos =
                    request.find("\r\n\r\n", requestHeaderStart);

                std::string requestHeader = request.substr(
                    requestHeaderStart, requestHeaderPos - requestHeaderStart);

                p.getRequestHeaders(requestHeader, req);

                if (req.headers.find("content-length") != req.headers.end()) {
                        std::string bodyBuffer =
                            request.substr(requestHeaderPos + 4);

                        std::vector<char> buff(8000, 0);
                        unsigned long clength =
                            std::stoi(req.headers["content-length"]);
                        while (bodyBuffer.size() < clength) {
                                ssize_t bread = read(clientFd.getFd(),
                                                     buff.data(), buff.size());

                                if (bread == 0) {
                                        return;
                                }

                                if (bread == -1) {
                                        std::cerr << "Failed reading http body"
                                                  << std::strerror(errno);
                                        return;
                                }

                                bodyBuffer.append(buff.begin(),
                                                  buff.begin() + bread);
                        }

                        leftover = bodyBuffer.substr(clength);
                        req.body = bodyBuffer.substr(0, clength);
                } else {
                        leftover = request.substr(requestHeaderPos + 4);
                }

                debug(req);

                std::cout << req.body.size() << std::endl;
                std::cout << req.headers["content-length"] << std::endl;

                r.match(req, res);

                res.version = req.version;
                const bool keepAlive = shouldKeepAlive(req);
                res.headers["connection"] = keepAlive ? "keep-alive" : "close";
                if (res.headers.find("content-length") == res.headers.end()) {
                        const size_t length = !res.body.empty()
                                                  ? res.body.size()
                                                  : res.bodyBytes.size();
                        res.headers["content-length"] = std::to_string(length);
                }

                std::string finalResponse = responseSerialization(res);
                if (!sendAll(clientFd.getFd(), finalResponse)) {
                        return;
                }

                if (!keepAlive) {
                        return;
                }
        }
}

void HTTP_SERVER::run() {

        while (true) {
                // NOTE: Now we accept the connection by using the
                // accept syscall which will pause the execution until
                // it gets a connection after getting the connection it
                // will create a new socket fd which will be responsible
                // for communication

                int fd = accept(serverSocket, nullptr, nullptr);

                // NOTE: We should not throw exception if ClientSocket
                // Failed because a server should keeps running even if
                // there exists one bad request instead we just logs the
                // details
                if (fd < 0) {
                        std::cerr << "Client socket Failed: "
                                  << std::strerror(errno);
                        continue;
                }

                ClientSocket clientFd(fd);

                // NOTE: Now comes the most important step for a
                // connection we need to spawn a thread so that it
                // handles the execution for the request The function in
                // thread will be responsible for everything the parsing
                // of the request the creation of request and response
                // objects NOTE: clientFd is move only we disabled the
                // copy constructor/assignment now for the main thread
                // clientFd will become -1 as defined in the move
                // constructor of ClientSocket socket
                //        std::thread t(connectionHandler,
                //        std::move(clientFd));

                // NOTE: it just let me change the static connectionHandler to
                //  non static so i can use the router member variable
                std::thread t([this, fd = std::move(clientFd)]() mutable {
                        this->connectionHandler(std::move(fd));
                });
                t.detach();
        }
}

// NOTE: This function will check the status codes and will take the
// filled value from route handler and is responsible to form a valid
// http-response object before sending
std::string HTTP_SERVER::responseSerialization(Response &res) {
        // NOTE: Made this static as we don't need it to spawn for every
        // thread
        static std::unordered_map<int, std::string> statusMap = {
            {200, "OK"},           {201, "Created"},
            {204, "No Content"},   {400, "Bad Request"},
            {401, "Unauthorized"}, {403, "Forbidden"},
            {404, "Not Found"},    {500, "Internal Server Error"},
            {502, "Bad Gateway"},  {503, "Service Unavailable"}};

        if (statusMap.find(res.statusCode) != statusMap.end()) {
                res.message = statusMap[res.statusCode];
        } else {
                res.message = statusMap[500];
        }

        auto generateResponse = [&]() -> std::string {
                std::string response;

                // NOTE: we can check the type of res.body if it's
                // vector<char> need to handle that
                if (!res.body.empty()) {
                        // true
                        response += res.version + " " +
                                    std::to_string(res.statusCode) + " " +
                                    res.message + "\r\n";

                        for (auto &[key, value] : res.headers) {
                                response += key + ": " + value + "\r\n";
                        }
                        response += "\r\n";

                        response += res.body;

                        return response;
                } else {
                        response += res.version + " " +
                                    std::to_string(res.statusCode) + " " +
                                    res.message + "\r\n";

                        for (auto &[key, value] : res.headers) {
                                response += key + ": " + value + "\r\n";
                        }
                        response += "\r\n";

                        std::string temp(res.bodyBytes.begin(),
                                         res.bodyBytes.end());
                        response += temp;

                        return response;
                }
        };
        return generateResponse();
}
