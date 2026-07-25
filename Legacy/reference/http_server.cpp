#include <algorithm>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>
// NOTE: Here we define the request object: ->
struct Request {
    // NOTE: These three are the request line the first line of the request
    // object
    std::string method;
    std::string url;
    std::string version;
    // NOTE: These are the header lines up until \r\n\r\n
    std::unordered_map<std::string, std::string> headers;
    // NOTE: This is the request body if the header lines contains
    // content-length
    std::string body;
};
// NOTE: This is For client socket it implements RAII
struct ClientSocket {
    int fd = -1;

    ClientSocket(int fd) : fd(fd) {}

    ~ClientSocket() {
        if (fd != -1) {
            close(fd);
        }
    }

    // NOTE: No copying because then two resource can own this file descriptor
    // and can simultaneously close the fd which can cause double delete which
    // is a bug.
    // We need to disable both because the compiler can still give the copy
    // assignment operator which can lead to undefined behaviour
    // They are not dependent on one another
    ClientSocket(const ClientSocket &) = delete;
    ClientSocket &operator=(const ClientSocket &) = delete;

    // NOTE: We have to allow moving to transfer the ownership as we don't want
    // any copy we just want to delete one and move the resource to other
    ClientSocket(ClientSocket &&other) noexcept : fd(other.fd) {
        // Old resource gives up the ownership
        other.fd = -1;
    }
    // NOTE: We declare both move constructor as well as move assignment
    // operator because it's the rule either don't define or define all
    ClientSocket &operator=(ClientSocket &&other) noexcept {
        if (this == &other)
            return *this;
        if (fd != -1)
            close(fd); // close the existing one first
        fd = other.fd;
        other.fd = -1;
        return *this;
    }
};

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

// NOTE: This is responsible for parsing the request -> refactor of classes and
// lld will come after we fix the code
void HTTP_Parser(std::string &mainBuffer, Request &req) {
    // NOTE: We have entered this function this means -> we have found the
    // \r\n\r\n So for parsing we need to find just the \r\n for the request
    // line and we have got the method, url, and version

    size_t requestLinePos = mainBuffer.find("\r\n");
    // FIX: If we did'nt find the position we can assume there is something
    // wrong with the request so we need to handle it - Later
    if (requestLinePos == std::string::npos) {
        // FIX: fix the return from here - empty for now
    }
    // NOTE: Now this means requestLinePos from 0 to requestLinePos we have the
    // request line

    std::string temp(mainBuffer.begin(), mainBuffer.begin() + requestLinePos);
    std::stringstream ss(temp);
    std::string token;
    std::vector<std::string> tokens;
    while (ss >> token) {
        tokens.push_back(token);
    }
    // NOTE: now tokens consist of the method, url , version
    if (tokens.empty() || tokens.size() != 3) {
        // return something is not right
    }
    req.method = tokens[0];
    req.url = tokens[1];
    req.version = tokens[2];

    // NOTE: now we need to start from the requestLinePos and go upto \r\n\r\n
    // Now comes the difficult task where we have to pares the header an

    // each header is first-> the header is upto "\r\n" and also each header has
    // a key and value and key is upto : and value start after this annoying
    // shit:)
    // From the end of requestLine + 2 we start the headers
    size_t pos = requestLinePos + 2;

    // NOTE: This is the function to handle front and back trailing spaces in
    // the request headers

    auto trim = [](std::string str) -> std::string {
        if (str.empty()) {
            return "";
        }
        size_t start = 0, end = str.size() - 1;

        while (start <= end && str[start] == ' ')
            start++;
        while (end >= start && str[end] == ' ')
            end--;

        return str.substr(start, end - start + 1);
    };

    while (pos < mainBuffer.size()) {
        // We find the position of colon
        size_t endPos = mainBuffer.find("\r\n", pos);

        // This create a sub string from pos of length r -l because + 1 will
        // might include \r  same as subarray
        std::string line = mainBuffer.substr(pos, endPos - pos);

        if (line.empty() || line == "\r")
            break;

        size_t colonPos = line.find(':');

        if (colonPos == std::string::npos)
            continue;
        // So from pos to colonPos we have the key and from colonPos + 1 to
        // endPos we have the headers

        size_t keyLength = colonPos;

        std::string key = line.substr(0, keyLength);
        std::string value = line.substr(colonPos + 1);

        // NOTE: Request headers can contain starting/trailing whitespaces -
        // remove them
        key = trim(key);
        value = trim(value);
        if (!key.empty() && !value.empty()) {
            req.headers[key] = value;
        }
        pos = endPos + 2;
    }

    // TODO: After this only body remains if content-length exists in headers
    // then we have to read -> that happens in the caller of parser
}
class HTTP_SERVER {
    // NOTE: Socket file descriptor we initialize it to -1 so that we won't
    // accidently call the close syscall on the invalid fd
    int serverSocket = -1;
    sockaddr_in serverSocketAddress;

  public:
    // NOTE: Contructor responsible for initializing the http server
    HTTP_SERVER(int PORT) {
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
    ~HTTP_SERVER() {
        if (serverSocket != -1) {
            close(serverSocket);
        }
    }

    // NOTE: We disable copy constructor as well as assignment for our
    // server because of the same reason we don't want it to copy a server
    // object and move (Contructor as well as assignment are not needed
    // because we don't want to move ownership of our server it will be one
    // for the whole lifetime of the program
    HTTP_SERVER(const HTTP_SERVER &) = delete;
    HTTP_SERVER &operator=(const HTTP_SERVER &) = delete;

    // NOTE: Why we make this static because we cannot pass a member
    // variable function in other functions and static makes sense here
    // because it does not have a this pointer which means its not for the
    // current object its for the whole class to share which means every
    // thread can access it with its own call stack and can run
    // simultaneously
    static void connectionHandler(ClientSocket clientFd) {
        // NOTE: We need to make the connection open and for that if the
        // read syscall return 0 then we can close the connection or the
        // http-request contains the connection close header line close the
        // connection Now we create a main buffer which will be responsible
        // for getting all the http-request and a temp buffer which will be
        // responsible for getting all the bytes from the read syscall and
        // appending it to the main buffer
        std::string mainBuffer;
        std::vector<char> requestBuffer(8000, 0);

        // FIX: This is a temporary response string delete this->
        std::string tempResponse = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Content-Length: 13\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Hello, world!";
        Request req;
        while (true) {
            // NOTE: First we need a buffer to store the http-request size -
            // 8kb
            // TODO: Add enums for the size

            ssize_t bytesRead =
                read(clientFd.fd, requestBuffer.data(), requestBuffer.size());

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
            if (headerPos != std::string::npos) {
                // NOTE: This means we found the headers we can just merge
                // the temp buffer to mainBuffer and break the loop will read
                // the body outside
                mainBuffer.append(requestBuffer.begin(),
                                  requestBuffer.begin() + bytesRead);
                break;
            } else {
                // We need to read again and again until we found the
                // \r\n\r\n
                continue;
            }
        }
        // NOTE: Now we have to parse the main to check if that
        // contains the content-length if yes then we need to read
        // the body too else we just break the loop
        HTTP_Parser(mainBuffer, req);

        // NOTE: After parsing we will have the headers now can check
        // for body but we need to append the mainBuffer to bodyBuffer as it can
        // contain some bytes after \r\n\r\n
        // Simply find the \r\n\r\n and then from that position to end we got
        // the remaining bytes

        size_t bodyPos = mainBuffer.find("\r\n\r\n");

        // FIX: Bug in this -> not printing the body
        if (req.headers.find("Content-Length") != req.headers.end()) {
            // NEED TO read again until buffer size is == content-length
            std::string bodyBuffer;
            std::vector<char> buf(8000, 0);
            while (bodyBuffer.size() <
                   std::stoi(req.headers["Content-Length"])) {

                ssize_t bRead = read(clientFd.fd, buf.data(), buf.size());

                if (bRead == 0) {
                    break;
                }

                if (bRead == -1) {
                    std::cerr << "Failed reading the http body "
                              << std::strerror(errno);
                    return;
                }

                // We got bytesRead - append them to req.body
                bodyBuffer.append(buf.begin(), buf.begin() + bRead);
            }
            req.body = bodyBuffer;

            std::cout << "bodyBuffer" << std::endl;
            std::cout << bodyBuffer << std::endl;
        }

        // FIX: This function is just for debugging purposes -> delete later
        debug(req);

        send(clientFd.fd, tempResponse.c_str(), tempResponse.size(), 0);
    }
    void run() {
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
            std::thread t(connectionHandler, std::move(clientFd));

            t.detach();
        }
    }
};

int main() {
    int PORT = 8989;
    HTTP_SERVER app(PORT);
    app.run();
}
