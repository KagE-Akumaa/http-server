#include "Http_Server.hpp"
#include "router.hpp"
#include <iostream>

int main() {
    int port = 8989;
    Router myRouter;
    myRouter.get("/", [](const Request &req, Response &res) {
        res.status(900);
        res.setContentType("application/json");

        res.setBody(R"({
  "users": [
    {
      "id": 1,
      "name": "Mukul",
      "email": "mukul@example.com"
    },
    {
      "id": 2,
      "name": "Aarav",
      "email": "aarav@example.com"
    },
    {
      "id": 3,
      "name": "Riya",
      "email": "riya@example.com"
    }
  ]
})");
    });

    HTTP_SERVER app(port, myRouter);
    app.run();
};
