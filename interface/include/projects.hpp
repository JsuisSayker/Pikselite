#pragma once

#include <chrono>
#include <filesystem>
#include <string>

namespace projects
{
    struct Project
    {
        std::string name = "New Project";
        std::filesystem::path path;
        std::chrono::system_clock::time_point lastOpened = std::chrono::system_clock::now();
    };
}