#include <iostream>
#include "Info.hpp"

Info::Info()
{
    colors["red"] = "\033[1;31m";
    colors["green"] = "\033[1;32m";
    colors["yellow"] = "\033[1;33m";
    colors["blue"] = "\033[1;34m";
    colors["magenta"] = "\033[1;35m";
    colors["cyan"] = "\033[1;36m";
    colors["white"] = "\033[1;37m";
    colors["default"] = "\033[0m";

    print = [&](std::string msg, std::string color) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << colors[color] << msg << colors["default"] << std::endl;
    };
}

Info::~Info()
{
}

Info info;