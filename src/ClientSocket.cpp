#include "ClientSocket.hpp"
#include <unistd.h>

// Contructor
ClientSocket::ClientSocket(int fd) : fd(fd) {}

// Destructor
ClientSocket::~ClientSocket() {
    if (fd != -1) {
        close(fd);
    }
}

// Move Contructor
ClientSocket::ClientSocket(ClientSocket &&other) noexcept : fd(other.fd) {
    other.fd = -1;
}

ClientSocket &ClientSocket::operator=(ClientSocket &&other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (fd != -1)
        close(fd);
    fd = other.fd;
    other.fd = -1;
    return *this;
}

int ClientSocket::getFd() const { return fd; }
