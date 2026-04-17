#include "Http_Server.hpp"
#include "router.hpp"
#include <iostream>

int main() {
    int port = 8989;
    Router myRouter;
    myRouter.get("/", [](const Request &req, Response &res) {
        std::cout << "HEllo from get" << std::endl;
    });

    HTTP_SERVER app(port, myRouter);
    app.run();
};
