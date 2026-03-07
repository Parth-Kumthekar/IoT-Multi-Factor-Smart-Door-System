#include "api_server.hpp"
#include "httplib.h"
#include <chrono>
#include <iostream>
#include <string>

namespace {
    long long nowMs() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
    }

    std::string extractUserFromJson(const std::string& body) {
        const std::string key = "\"user\"";
        std::size_t keyPos = body.find(key);
        if (keyPos == std::string::npos) {
            return "unknown";
        }

        std::size_t colonPos = body.find(':', keyPos);
        if (colonPos == std::string::npos) {
            return "unknown";
        }

        std::size_t firstQuote = body.find('"', colonPos + 1);
        if (firstQuote == std::string::npos) {
            return "unknown";
        }

        std::size_t secondQuote = body.find('"', firstQuote + 1);
        if (secondQuote == std::string::npos) {
            return "unknown";
        }

        return body.substr(firstQuote + 1, secondQuote - firstQuote - 1);
    }
}

ApiServer::ApiServer(SystemState& state) : state_(state) {}

void ApiServer::run(const char* host, int port) {
    httplib::Server server;

    server.Get("/api/status", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(state_.toJson(), "application/json");
        });

    server.Post("/api/auth", [this](const httplib::Request& req, httplib::Response& res) {
        std::string user = extractUserFromJson(req.body);
        state_.setLastAuth(user, nowMs());

        std::string body =
            "{"
            "\"ok\":true,"
            "\"message\":\"Authorization recorded\","
            "\"user\":\"" + user + "\""
            "}";

        res.set_content(body, "application/json");
        });

    server.Post("/api/alarm/reset", [this](const httplib::Request&, httplib::Response& res) {
        state_.setAlarmActive(false);

        std::string body =
            "{"
            "\"ok\":true,"
            "\"alarm_active\":false"
            "}";

        res.set_content(body, "application/json");
        });

    server.Post("/api/system/arm", [this](const httplib::Request&, httplib::Response& res) {
        state_.setArmed(true);

        std::string body =
            "{"
            "\"ok\":true,"
            "\"armed\":true"
            "}";

        res.set_content(body, "application/json");
        });

    server.Post("/api/system/disarm", [this](const httplib::Request&, httplib::Response& res) {
        state_.setArmed(false);

        std::string body =
            "{"
            "\"ok\":true,"
            "\"armed\":false"
            "}";

        res.set_content(body, "application/json");
        });

    server.Post("/api/test/door/open", [this](const httplib::Request&, httplib::Response& res) {
        state_.setDoorOpen(true);

        std::string body =
            "{"
            "\"ok\":true,"
            "\"door_open\":true"
            "}";

        res.set_content(body, "application/json");
        });

    server.Post("/api/test/door/close", [this](const httplib::Request&, httplib::Response& res) {
        state_.setDoorOpen(false);

        std::string body =
            "{"
            "\"ok\":true,"
            "\"door_open\":false"
            "}";

        res.set_content(body, "application/json");
        });

    std::cout << "[API] Listening on http://" << host << ":" << port << std::endl;
    server.listen(host, port);
}