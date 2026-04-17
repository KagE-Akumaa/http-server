#pragma once

#include "ClientSocket.hpp"
#include "router.hpp"
#include <netinet/in.h>
class HTTP_SERVER {
    int serverSocket = -1;
    sockaddr_in serverSocketAddress;

    Router &r;

  public:
    HTTP_SERVER(int PORT, Router &r);
    ~HTTP_SERVER();
    HTTP_SERVER(const HTTP_SERVER &) = delete;
    HTTP_SERVER &operator=(const HTTP_SERVER &) = delete;

    void run();

    void connectionHandler(ClientSocket clientFd);
};
