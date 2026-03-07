#pragma once
#include "system_state.hpp"

class ApiServer {
public:
    explicit ApiServer(SystemState& state);

    void run(const char* host, int port);

private:
    SystemState& state_;
};
