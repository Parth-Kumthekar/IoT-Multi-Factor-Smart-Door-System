#include "NFCReader.hpp"
#include "OutputController.hpp"

int main() {
    NFCReader nfc("/dev/serial0", 9600);

    if (!nfc.init()) {
        std::cerr << "UART init failed\n";
        return -1;
    }

    OutputController output;
    output.init();

    const std::string VALID_UID = "12345678";  // change this

    while (true) {
        std::string uid = nfc.readUID();

        if (!uid.empty()) {
            std::cout << "UID: " << uid << std::endl;

            if (uid == VALID_UID) {
                output.setAccessGranted();
            } else {
                output.setAccessDenied();
            }
        }
    }

    return 0;
}