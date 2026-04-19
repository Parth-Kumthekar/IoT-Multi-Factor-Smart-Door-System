#pragma once
#include <string>
#include <vector>

class AccessController {
public:
    bool check(const std::string &uid);

private:
    std::vector<std::string> allowed = {
        "040ADB8A7111",
        "04361B6ABB77"
    };
};