#pragma once

class ClientSocket {
    int fd = -1;

  public:
    ClientSocket(int fd);

    ~ClientSocket();

    ClientSocket(const ClientSocket &) = delete;
    ClientSocket &operator=(const ClientSocket &) = delete;

    ClientSocket(ClientSocket &&other) noexcept;

    ClientSocket &operator=(ClientSocket &&other) noexcept;

    int getFd() const;
};
