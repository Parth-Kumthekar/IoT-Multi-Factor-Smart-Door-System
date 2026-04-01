#include "NFCReader.hpp"
#include "OutputHandler.hpp"

int main() {
    NFCReader nfc;
    OutputHandler outputs(6, 26, 19, 13); // Pins for Relay, Green, Red, Buzzer

    if (!nfc.connect() || !outputs.init()) {
        std::cerr << "Hardware Failed!" << std::endl;
        return 1;
    }

    while (true) {
        std::string cardID = nfc.scanCard();
        
        if (!cardID.empty()) {
            std::cout << "Detected Card: " << cardID << std::endl;
            if (cardID == "046732ca") { // Your specific card UID
                outputs.setAccessGranted();
            } else {
                outputs.setAccessDenied();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}