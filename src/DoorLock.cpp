#include "DoorLock.h"

DoorLock::DoorLock(int gpioPin) : gpioPin_(gpioPin) {
    // Pi 5 uses gpiochip4 (RP1); fall back to gpiochip0 for Pi 4/3
    chip_ = gpiod_chip_open_by_name("gpiochip4");
    if (!chip_) {
        std::cerr << "[DoorLock] gpiochip4 not found, trying gpiochip0\n";
        chip_ = gpiod_chip_open_by_name("gpiochip0");
    }
    if (!chip_)
        throw std::runtime_error("[DoorLock] Failed to open any GPIO chip");

    line_ = gpiod_chip_get_line(chip_, gpioPin_);
    if (!line_)
        throw std::runtime_error("[DoorLock] Failed to get GPIO line");

    if (gpiod_line_request_output(line_, "door_lock", 0) < 0)
        throw std::runtime_error("[DoorLock] Failed to request GPIO as output");

    std::cout << "[DoorLock] Initialised on BCM pin " << gpioPin_ << "\n";
}

DoorLock::~DoorLock() {
    if (line_) { gpiod_line_set_value(line_, 0); gpiod_line_release(line_); }
    if (chip_)   gpiod_chip_close(chip_);
}

bool DoorLock::unlock(const std::string& personName) {
    auto now = std::chrono::steady_clock::now();
    if (!firstUnlock_) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
                       (now - lastUnlock_).count();
        if (elapsed < kCooldownMs) return false;
    }

    firstUnlock_ = false;
    lastUnlock_  = now;
    locked_.store(false);

    std::cout << "[DoorLock] ACCESS GRANTED — " << personName << "\n";
    gpiod_line_set_value(line_, 1);  // energise relay → unlock solenoid

    std::thread([this]{ lockAfterDelay(kUnlockDurationMs); }).detach();
    return true;
}

void DoorLock::lockAfterDelay(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    gpiod_line_set_value(line_, 0);
    locked_.store(true);
    std::cout << "[DoorLock] Door re-locked\n";
}