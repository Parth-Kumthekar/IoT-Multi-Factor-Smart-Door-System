#include "AsyncLogger.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>

void AsyncLogger::processLogs() {
    // Open the CSV file in Append mode
    std::ofstream dbFile("security_audit.csv", std::ios::app);

    while (running_) {
        std::string msg;
        if (logQueue_.waitAndPop(msg)) {
            // 1. Get current time
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto ts = std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");

            // 2. Print to Terminal (for you to see)
            std::cout << "[" << ts << "] " << msg << std::endl;

            // 3. Write to Local CSV (The "Database" copy)
            if (dbFile.is_open()) {
                dbFile << ts << "," << msg << "\n";
                dbFile.flush(); // Ensure it writes to disk immediately
            }
        }
    }
}