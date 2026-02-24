#define CROW_USE_ASIO
#define ASIO_STANDALONE

#include "crow.h"
#include "nlohmann/json.hpp"
#include "notice_functions.h"
#include <iostream>

using json = nlohmann::json;

int main()
{
    crow::SimpleApp app;

    // 1. GET ALL NOTICES
    CROW_ROUTE(app, "/notices").methods("GET"_method)
    ([](){
        auto notices = getAllNotices();
        json j = json::array();

        for (auto& n : notices) {
            j.push_back({
                {"id", n.id},
                {"title", n.title},
                {"content", n.content},
                {"author", n.author}
            });
        }

        crow::response res(j.dump());
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Headers", "Content-Type"); // Added for safety
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // 2. POST A NEW NOTICE & OPTIONS
    CROW_ROUTE(app, "/notices").methods("POST"_method, "OPTIONS"_method)
    ([](const crow::request& req){
    crow::response res;
    // Add these three headers to every response in this route
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");

    if (req.method == "OPTIONS"_method) {
        res.code = 200;
        return res;
    }

    try {
        auto body = json::parse(req.body);
        Notice n = { body["id"], body["title"], body["content"], body["author"] };

        if (addNotice(n)) {
            res.code = 200;
            res.body = "OK";
        } else {
            res.code = 400;
            res.body = "Failed to save";
        }
    } catch (...) {
        res.code = 400;
        res.body = "JSON Error";
    }
    return res;
});
    // 3. DELETE A NOTICE
    CROW_ROUTE(app, "/notices/<int>").methods("DELETE"_method, "OPTIONS"_method)
    ([](const crow::request& req, int id){
        crow::response res;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");

        if (req.method == "OPTIONS"_method) {
            res.code = 200;
            return res;
        }

        if (deleteNotice(id)) {
            res.code = 200;
            res.body = "Deleted";
        } else {
            res.code = 404;
            res.body = "Notice Not Found";
        }
        return res;
    });

    std::cout << "Server running at http://localhost:18080" << std::endl;
    app.port(18080).multithreaded().run();
}
