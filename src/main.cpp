#include "system_state.hpp"
#include "api_server.hpp"
#include <iostream>

int main() {
    SystemState state;

    state.setArmed(true);
    state.setDoorOpen(false);
    state.setAlarmActive(false);

    ApiServer server(state);

    std::cout << "[MAIN] Starting API server..." << std::endl;
    server.run("0.0.0.0", 8080);

    return 0;
}