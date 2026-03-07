#pragma once
#include <mutex>
#include <string>
#include <sstream>

class SystemState {
public:
    void setArmed(bool v);
    bool isArmed() const;

    void setDoorOpen(bool v);
    bool isDoorOpen() const;

    void setAlarmActive(bool v);
    bool isAlarmActive() const;

    void setLastAuth(const std::string& user, long long timeMs);
    std::string getLastAuthUser() const;
    long long getLastAuthTime() const;

    void setLastIntrusionTime(long long timeMs);
    long long getLastIntrusionTime() const;

    std::string toJson() const;

private:
    mutable std::mutex mutex_;
    bool armed_ = true;
    bool doorOpen_ = false;
    bool alarmActive_ = false;
    std::string lastAuthUser_ = "";
    long long lastAuthTime_ = 0;
    long long lastIntrusionTime_ = 0;
};
