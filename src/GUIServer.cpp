#include "GUIServer.h"
#include "../include/Logger.h"
#include "../include/OverrideManager.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>

GUIServer::GUIServer(int port) : port_(port) {}

GUIServer::~GUIServer() { stop(); }

void GUIServer::start() {
    running_.store(true);
    thread_ = std::thread(&GUIServer::serve, this);
    std::cout << "[GUIServer] Listening on http://0.0.0.0:" << port_ << "\n";
}

void GUIServer::stop() {
    running_.store(false);
    if (serverFd_ >= 0) { close(serverFd_); serverFd_ = -1; }
    if (thread_.joinable()) thread_.join();
}

void GUIServer::serve() {
    serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port_);
    bind(serverFd_, (sockaddr*)&addr, sizeof(addr));
    listen(serverFd_, 5);

    while (running_.load()) {
        int client = accept(serverFd_, nullptr, nullptr);
        if (client < 0) break;

        char buf[4096] = {};
        read(client, buf, sizeof(buf)-1);
        std::string req(buf);
        std::string response = handleRequest(req);
        write(client, response.c_str(), response.size());
        close(client);
    }
}

std::string GUIServer::handleRequest(const std::string& req) {
    auto cors = "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Methods: GET, POST\r\n"
                "Access-Control-Allow-Headers: Content-Type\r\n";

    // GET /status
    if (req.find("GET /status") != std::string::npos) {
        std::string body = jsonStatus();
        std::ostringstream r;
        r << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
          << cors << "Content-Length: " << body.size() << "\r\n\r\n" << body;
        return r.str();
    }

    // GET /logs
    if (req.find("GET /logs") != std::string::npos) {
        std::string body = jsonLogs();
        std::ostringstream r;
        r << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
          << cors << "Content-Length: " << body.size() << "\r\n\r\n" << body;
        return r.str();
    }

    // POST /override/on
    if (req.find("POST /override/on") != std::string::npos) {
        OverrideManager::instance().enable("GUI");
        std::string body = "{\"ok\":true,\"override\":true}";
        std::ostringstream r;
        r << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
          << cors << "Content-Length: " << body.size() << "\r\n\r\n" << body;
        return r.str();
    }

    // POST /override/off
    if (req.find("POST /override/off") != std::string::npos) {
        OverrideManager::instance().disable("GUI");
        std::string body = "{\"ok\":true,\"override\":false}";
        std::ostringstream r;
        r << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
          << cors << "Content-Length: " << body.size() << "\r\n\r\n" << body;
        return r.str();
    }

    // POST /unlock
    if (req.find("POST /unlock") != std::string::npos) {
        Logger::instance().log("Admin", AccessMethod::MANUAL,
                               AccessResult::GRANTED, "Manual unlock via GUI");
        std::string body = "{\"ok\":true}";
        std::ostringstream r;
        r << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
          << cors << "Content-Length: " << body.size() << "\r\n\r\n" << body;
        return r.str();
    }

    // POST /note  — body: {"note":"..."}
    if (req.find("POST /note") != std::string::npos) {
        auto pos = req.find("\"note\":\"");
        std::string note = "Manual note";
        if (pos != std::string::npos) {
            pos += 8;
            auto end = req.find('"', pos);
            if (end != std::string::npos) note = req.substr(pos, end-pos);
        }
        Logger::instance().log("Admin", AccessMethod::NOTE,
                               AccessResult::GRANTED, note);
        std::string body = "{\"ok\":true}";
        std::ostringstream r;
        r << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
          << cors << "Content-Length: " << body.size() << "\r\n\r\n" << body;
        return r.str();
    }

    std::string body = "{\"error\":\"not found\"}";
    std::ostringstream r;
    r << "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\n"
      << cors << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    return r.str();
}

std::string GUIServer::jsonLogs() {
    const auto& entries = Logger::instance().entries();
    std::ostringstream j;
    j << "[";
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& e = entries[i];
        j << "{\"time\":\"" << e.timestamp << "\","
          <<  "\"person\":\"" << e.person << "\","
          <<  "\"method\":\"" << Logger::methodStr(e.method) << "\","
          <<  "\"result\":\"" << Logger::resultStr(e.result) << "\","
          <<  "\"note\":\"" << e.note << "\"}";
        if (i+1 < entries.size()) j << ",";
    }
    j << "]";
    return j.str();
}

std::string GUIServer::jsonStatus() {
    bool ov = OverrideManager::instance().isActive();
    std::ostringstream j;
    j << "{\"override\":" << (ov?"true":"false") << "}";
    return j.str();
}