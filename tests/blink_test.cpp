#include "OutputController.hpp"
#include <thread>
#include <chrono>

int main() {
    OutputController oc;
    if (!oc.init()) return 1;

    // Cycle through hardware to verify wiring
    std::cout << "Testing Red LED...\n";
    oc.setRedLed(true);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    oc.setRedLed(false);

    std::cout << "Testing Green LED...\n";
    oc.setGreenLed(true);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    oc.setGreenLed(false);

    std::cout << "Testing Buzzer...\n";
    oc.setBuzzer(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    oc.setBuzzer(false);

    return 0;
}