#pragma once
#include <atomic>
#include <csignal>
#include <iostream>

inline std::atomic<bool> gShutdown{false};

inline void installSignalHandlers() {
    auto handler = [](int sig) {
        std::cout << "\n[Signal] Caught signal " << sig
                  << " — shutting down cleanly\n";
        gShutdown.store(true);
    };
    std::signal(SIGINT,  handler);   // Ctrl+C
    std::signal(SIGTERM, handler);   // systemctl stop
#ifndef _WIN32
    std::signal(SIGHUP,  handler);   // terminal hangup
#endif
}