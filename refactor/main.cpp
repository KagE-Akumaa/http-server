#include "Http_Server.hpp"
#include "json.hpp"
#include "router.hpp"

using json = nlohmann::json;
int main() {
    int port = 8989;
    Router myRouter;
    myRouter.get("/", [](const Request &req, Response &res) {
        res.status(200);
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

    myRouter.post("/", [](const Request &req, Response &res) {
        auto header = req.headers.find("content-type");

        if (header != req.headers.end() &&
            header->second == "application/json") {
            try {
                json data = json::parse(req.body);
                std::string name = data["name"];
                std::string email = data["email"];

                res.status(200);
                res.setContentType("application/json");
                res.setBody(R"({"message" : "JSON recieved"})");
            } catch (const json::parse_error &e) {
                res.status(400);
                res.setContentType("application/json");
                res.setBody(R"({"error": "Invalid JSON"})");
            }
        } else {
            res.status(415);
            res.setBody("Expected application/json");
        }
    });
    HTTP_SERVER app(port, myRouter);
    app.run();
};
