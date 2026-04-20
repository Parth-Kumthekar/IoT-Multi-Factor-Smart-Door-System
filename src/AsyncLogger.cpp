#include "AsyncLogger.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

void AsyncLogger::processLogs() {
    // Open the CSV file in Append mode
    // Path is relative to where you run the executable
    std::ofstream dbFile("security_audit.csv", std::ios::app);

    // Write a CSV Header if the file is new/empty
    if (dbFile.tellp() == 0) {
        dbFile << "Timestamp,Category,Details\n";
    }

    while (running_) {
        std::string rawMsg;
        
        // This blocks efficiently until a new log entry arrives
        if (logQueue_.waitAndPop(rawMsg)) {
            
            // 1. Generate Timestamp
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            
            std::ostringstream timeStream;
            timeStream << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
            std::string ts = timeStream.str();

            // 2. Simple Parsing (Split "CATEGORY: Message" into two CSV columns)
            std::string category = "INFO";
            std::string detail = rawMsg;
            size_t colonPos = rawMsg.find(": ");
            
            if (colonPos != std::string::npos) {
                category = rawMsg.substr(0, colonPos);
                detail = rawMsg.substr(colonPos + 2);
            }

            // 3. Print to Terminal with color-coding (Optional but helpful)
            std::cout << "[" << ts << "] [" << category << "] " << detail << std::endl;

            // 4. Write to Local CSV (The "Local Database" copy)
            if (dbFile.is_open()) {
                // We wrap detail in quotes in case it contains a comma itself
                dbFile << ts << "," << category << ",\"" << detail << "\"\n";
                
                // Flush ensures the data is written to the SD card immediately
                // preventing loss if the Pi is unplugged.
                dbFile.flush(); 
            }
        }
    }

    if (dbFile.is_open()) {
        dbFile.close();
    }
}