#include "DoorController.hpp"
#include "AccessController.hpp"
#include "OutputController.hpp"
#include "NFCReader.hpp"
#include <vector>
#include <iostream>

int main() {
    std::vector<std::string> authorizedUIDs = {"04AABBCC", "11223344", "55667788"};

    AccessController accessController(authorizedUIDs);
    OutputController outputController;
    NFCReader nfcReader;
    DoorController doorController(4, accessController, outputController, nfcReader);

    outputController.init();
    doorController.initialize();

    std::cout << "System running. Press Ctrl+C to exit." << std::endl;
    while (true) std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}