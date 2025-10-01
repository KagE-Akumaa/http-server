#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
const int PORT = 9000;
using namespace std;
int main() {
    // Inititalize the socket
    // AF_INET -  protocol family, tcp udp etc,
    int nSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (nSocket < 0) {
        cout << "Failed to open a socket" << endl;

        exit(EXIT_FAILURE);
        return -1;

    } else {
        cout << "Opening a socket Successfull !" << endl << nSocket;
    }
    sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;

    if (bind(nSocket, (sockaddr*)&srv, sizeof(srv)) < 0) {
        cout << "Failed to bind the socket" << endl;
        exit(EXIT_FAILURE);
        return -1;
    } else {
        cout << endl << "Successfully bind to socket to the PORT";
    }
    // Listen on the PORT Hold atmost 10 connections in the queue
    if (listen(nSocket, 10) < 0) {
        cout << endl << "Failed to listen on PORT !";
        exit(EXIT_FAILURE);
        return -1;
    } else {
        cout << endl << "Started listening on PORT ";
    }
    // Grab a connection from the queue
    // create a new socket for client standard way

    while (true) {
        // For every request that comes to the sever will come to the client side socket and from
        // that we can read the request info and put that into a buffer
        int client_fd = accept(nSocket, nullptr, nullptr);
        char buffer[30000] = {0};

        read(client_fd, buffer, 30000);
        cout << endl << buffer << endl;

        // Now create a response which we need to give to the request in HTTP format
        string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 46\r\n"
            "\r\n"
            "<html><body><h1>Hello from C++ server</h1></body></html>";
        // NOT SURE         // Now write this into the client socket descriptor which will take the
        // response back to the client

        write(client_fd, response.c_str(), response.size());
        close(client_fd);
    }
    close(nSocket);
}
