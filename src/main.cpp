#include <iostream>
#include "NFCReader.hpp"
#include "OutputController.hpp"
#include "AccessController.hpp"

int main()
{
    NFCReader nfc;
    OutputController out;
    AccessController auth;

    if (!nfc.init())
    {
        std::cout << "UART init failed\n";
        return 1;
    }

    out.init();

    std::cout << "Smart Door Running...\n";

    while (true)
    {
        std::string uid = nfc.readUID();

        if (uid.empty()) continue;

        std::cout << "UID: " << uid << std::endl;

        if (auth.check(uid))
        {
            std::cout << "ACCESS GRANTED\n";
            out.granted();
        }
        else
        {
            std::cout << "ACCESS DENIED\n";
            out.denied();
        }
    }
}