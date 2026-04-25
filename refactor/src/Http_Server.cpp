#include "Http_Server.hpp"
#include "Http_Parser.hpp"
#include "Http_Request.hpp"
#include "router.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
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
        std::cout << "Key: " << it.first << " " << "Value: " << it.second
                  << std::endl;
    }

    std::cout << "Logging request Body ---> " << std::endl;

    std::cout << req.body << std::endl;
}

HTTP_SERVER::HTTP_SERVER(int PORT, Router &r) : r(r) {

    // Get the socket fd we chose the tcp socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error(std::string("Failed to create a socket: ") +
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
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) <
        0) {
        throw std::runtime_error(std::string("Failed to add socket option: ") +
                                 std::strerror(errno));
    }

    // Now we need to bind the socket to the port
    if (bind(serverSocket, (sockaddr *)&serverSocketAddress,
             sizeof(serverSocketAddress)) < 0) {
        throw std::runtime_error(std::string("Failed to bind the socket: ") +
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
    //

    // NOTE: We need to make the connection open and for that if the
    // read syscall return 0 then we can close the connection or the
    // http-request contains the connection close header line close the
    // connection Now we create a main buffer which will be responsible
    // for getting all the http-request and a temp buffer which will be
    // responsible for getting all the bytes from the read syscall and
    // appending it to the main buffer
    std::string request;
    std::vector<char> requestBuffer(8000, 0);

    while (true) {
        // NOTE: First we need a buffer to store the http-request size -
        // 8kb
        // TODO: Add enums for the size

        ssize_t bytesRead =
            read(clientFd.getFd(), requestBuffer.data(), requestBuffer.size());

        if (bytesRead == 0) {
            // NOTE: Need to close the connection
            // TODO: how to close the connection?
            return;
        }
        if (bytesRead == -1) {
            // NOTE: Error so can we throw here?
            // We don't throw here because our thread is using detach so
            // it will directly goes to std::terminate which will
            // terminate the whole server instead we just logs the error
            std::cerr << "Failed reading the http-request: "
                      << std::strerror(errno);
            return;
        }

        // If we reached here then we got bytesRead
        std::string temp(requestBuffer.begin(),
                         requestBuffer.begin() + bytesRead);
        size_t headerPos = temp.find("\r\n\r\n");
        if (headerPos == std::string::npos) {
            // We need to read again and again until we found the
            // \r\n\r\n
            continue;

        } else {
            // NOTE: This means we found the headers we can just merge
            // the temp buffer to request and break the loop will read
            // the body outside
            request.append(requestBuffer.begin(),
                           requestBuffer.begin() + bytesRead);
            break;
        }
    }
    // NOTE: Now we have to parse the main to check if that
    // contains the content-length if yes then we need to read
    // the body too else we just break the loop

    Request req;
    Response res;
    Parser p;

    size_t requestLinePos = request.find("\r\n");

    std::string requestLine = request.substr(0, requestLinePos);

    p.getRequestLine(requestLine, req);

    // Request Line are successfully parsed

    // Now request header :->

    // requestLine + 1 -> "\r\n\r\n"

    size_t requestHeaderStart = requestLinePos + 2;
    size_t requestHeaderPos = request.find("\r\n\r\n", requestHeaderStart);

    std::string requestHeader = request.substr(
        requestHeaderStart, requestHeaderPos - requestHeaderStart);

    p.getRequestHeaders(requestHeader, req);

    // NOTE: We can just simply get all the body first and then we can parse
    // it

    // Now comes the request body only if content-length exits
    if (req.headers.find("content-length") != req.headers.end()) {
        // NOTE: We might have read some of the body after '\r\n\r\n'
        // Remember to add 4 bytes for the \r\n\r\n :)
        std::string bodyBuffer = request.substr(requestHeaderPos + 4);

        // FIX: Remove these magic numbers once this works :)
        std::vector<char> buff(8000, 0);
        int clength = std::stoi(req.headers["content-length"]);
        while (bodyBuffer.size() < clength) {

            ssize_t bread = read(clientFd.getFd(), buff.data(), buff.size());

            if (bread == 0) {
                break;
            }

            if (bread == -1) {
                std::cerr << "Failed reading http body" << std::strerror(errno);
                return;
            }

            // We have bytes read - append them
            bodyBuffer.append(buff.begin(), buff.begin() + bread);
        }

        req.body = bodyBuffer;
    }

    // FIX: This function is just for debugging purposes -> delete later
    debug(req);

    std::cout << req.body.size() << std::endl;
    std::cout << req.headers["content-length"] << std::endl;

    r.match(req, res);

    res.version = req.version;
    std::string finalResponse = responseSerialization(res);
    send(clientFd.getFd(), finalResponse.c_str(), finalResponse.size(), 0);
}

void HTTP_SERVER::run() {

    while (true) {
        // NOTE: Now we accept the connection by using the accept
        // syscall which will pause the execution until it gets a
        // connection after getting the connection it will create a new
        // socket fd which will be responsible for communication

        int fd = accept(serverSocket, nullptr, nullptr);

        // NOTE: We should not throw exception if ClientSocket Failed
        // because a server should keeps running even if there exists
        // one bad request instead we just logs the details
        if (fd < 0) {
            std::cerr << "Client socket Failed: " << std::strerror(errno);
            continue;
        }

        ClientSocket clientFd(fd);

        // NOTE: Now comes the most important step for a connection we
        // need to spawn a thread so that it handles the execution for
        // the request The function in thread will be responsible for
        // everything the parsing of the request the creation of request
        // and response objects NOTE: clientFd is move only we disabled
        // the copy constructor/assignment now for the main thread
        // clientFd will become -1 as defined in the move constructor of
        // ClientSocket socket
        //        std::thread t(connectionHandler, std::move(clientFd));

        // FIX: Don't know what it does it just works for now :)
        // it just let me change the static connectionHandler to non static so i
        // can use the router member variable
        std::thread t([this, fd = std::move(clientFd)]() mutable {
            this->connectionHandler(std::move(fd));
        });
        t.detach();
    }
}

// NOTE: This function will check the status codes and will take the filled
// value from route handler and is responsible to form a valid http-response
// object before sending
std::string HTTP_SERVER::responseSerialization(Response &res) {
    // NOTE: Made this static as we don't need it to spawn for every thread
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

        // NOTE: we can check the type of res.body if it's vector<char> need to
        // handle that
        if (!res.body.empty()) {
            // true
            response += res.version + " " + std::to_string(res.statusCode) +
                        " " + res.message + "\r\n";

            for (auto &[key, value] : res.headers) {
                response += key + ": " + value + "\r\n";
            }
            response += "\r\n";

            response += res.body;

            return response;
        } else {
            response += res.version + " " + std::to_string(res.statusCode) +
                        " " + res.message + "\r\n";

            for (auto &[key, value] : res.headers) {
                response += key + ": " + value + "\r\n";
            }
            response += "\r\n";

            std::string temp(res.bodyBytes.begin(), res.bodyBytes.end());
            response += temp;

            return response;
        }
    };
    return generateResponse();
}
