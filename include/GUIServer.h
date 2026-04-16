#pragma once
#include <thread>
#include <atomic>
#include <string>

class GUIServer {
public:
    explicit GUIServer(int port = 8080);
    ~GUIServer();
    void start();
    void stop();

private:
    void serve();
    std::string handleRequest(const std::string& req);
    std::string jsonLogs();
    std::string jsonStatus();

    int               port_;
    int               serverFd_ = -1;
    std::atomic<bool> running_{false};
    std::thread       thread_;
};