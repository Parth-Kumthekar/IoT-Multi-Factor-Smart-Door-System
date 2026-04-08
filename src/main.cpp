#include "../include/DoorAlarmSystem.h"
#include <iostream>
#include <string>

static void printHelp()
{
    std::cout << "\nCommands:\n"
              << "  arm        -> arm system\n"
              << "  disarm     -> disarm system\n"
              << "  door_open  -> simulate door open\n"
              << "  door_close -> simulate door close\n"
              << "  nfc_ok     -> simulate valid NFC auth\n"
              << "  app_ok     -> simulate valid APP auth\n"
              << "  status     -> print system status\n"
              << "  help       -> show commands\n"
              << "  quit       -> exit program\n\n";
}

int main()
{
    DoorAlarmSystem system;
    system.start();

    printHelp();

    std::string cmd;
    while (true)
    {
        std::cout << "> ";
        if (!std::getline(std::cin, cmd))
        {
            break;
        }

        if (cmd == "arm")
        {
            system.postEvent(EventType::ArmSystem, "console");
        }
        else if (cmd == "disarm")
        {
            system.postEvent(EventType::DisarmSystem, "console");
        }
        else if (cmd == "door_open")
        {
            system.postEvent(EventType::DoorOpened, "doorSensor");
        }
        else if (cmd == "door_close")
        {
            system.postEvent(EventType::DoorClosed, "doorSensor");
        }
        else if (cmd == "nfc_ok")
        {
            system.postEvent(EventType::AuthorizedByNfc, "nfcReader");
        }
        else if (cmd == "app_ok")
        {
            system.postEvent(EventType::AuthorizedByApp, "apiServer");
        }
        else if (cmd == "status")
        {
            system.postEvent(EventType::PrintStatus, "console");
        }
        else if (cmd == "help")
        {
            printHelp();
        }
        else if (cmd == "quit")
        {
            break;
        }
        else if (!cmd.empty())
        {
            std::cout << "Unknown command. Type 'help'.\n";
        }
    }

    system.stop();
    return 0;
}