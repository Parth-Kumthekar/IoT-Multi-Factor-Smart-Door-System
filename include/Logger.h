#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

enum class AccessResult { GRANTED, DENIED, OVERRIDE };
enum class AccessMethod  { FACE_ID, NFC, MANUAL, OVERRIDE, NOTE };

struct LogEntry {
    std::string  timestamp;
    std::string  person;
    AccessMethod method;
    AccessResult result;
    std::string  note;
};

class Logger {
public:
    static Logger& instance() { static Logger l; return l; }

    void log(const std::string& person,
             AccessMethod method,
             AccessResult result,
             const std::string& note = "") {
        std::lock_guard<std::mutex> lk(mutex_);
        LogEntry e{ nowString(), person, method, result, note };
        entries_.push_back(e);
        writeCSVRow(e);
        std::cout << "[LOG] " << e.timestamp
                  << " | " << e.person
                  << " | " << methodStr(method)
                  << " | " << resultStr(result);
        if (!note.empty()) std::cout << " | " << note;
        std::cout << "\n";
    }

    const std::vector<LogEntry>& entries() const { return entries_; }

    static std::string methodStr(AccessMethod m) {
        switch(m){
            case AccessMethod::FACE_ID:  return "Face ID";
            case AccessMethod::NFC:      return "NFC";
            case AccessMethod::MANUAL:   return "Manual";
            case AccessMethod::OVERRIDE: return "Override";
            case AccessMethod::NOTE:     return "Note";
        }
        return "Unknown";
    }
    static std::string resultStr(AccessResult r) {
        switch(r){
            case AccessResult::GRANTED:  return "Granted";
            case AccessResult::DENIED:   return "Denied";
            case AccessResult::OVERRIDE: return "Override";
        }
        return "Unknown";
    }

private:
    Logger() {
        csv_.open("database/access_log.csv", std::ios::app);
        if (csv_.tellp() == 0)
            csv_ << "Time,Person,Method,Result,Note\n";
    }

    void writeCSVRow(const LogEntry& e) {
        csv_ << '"' << e.timestamp << "\","
             << '"' << e.person    << "\","
             << '"' << methodStr(e.method) << "\","
             << '"' << resultStr(e.result) << "\","
             << '"' << e.note      << "\"\n";
        csv_.flush();
    }

    static std::string nowString() {
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        std::ostringstream ss;
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::mutex           mutex_;
    std::vector<LogEntry> entries_;
    std::ofstream        csv_;
};