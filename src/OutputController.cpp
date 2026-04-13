#include "OutputController.hpp"
#include <thread>
#include <chrono>

OutputController::OutputController() : ledGreen(17, true), ledRed(27, true), buzzer(22, true) {}

void OutputController::init() {
    ledGreen.start(0);
    ledRed.start(0);
    buzzer.start(0);
}

void OutputController::setAccessGranted() {
    ledGreen.setValue(1);
    ledRed.setValue(0);
    buzzer.setValue(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    buzzer.setValue(0);
}

void OutputController::setAccessDenied() {
    ledGreen.setValue(0);
    ledRed.setValue(1);
    buzzer.setValue(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    buzzer.setValue(0);
}