#pragma once

#include <cstring>
#include <string>

struct BuildSettings
{
    std::string gameTitle = "Pikselite Game";
    int windowWidth = 1280;
    int windowHeight = 720;
    std::string targetName = "Game";
    std::string outputPath;
    std::string scenePath;

    // Char buffers for safe ImGui InputText editing
    char gameTitleBuf[256] = {};
    char targetNameBuf[256] = {};

    void syncToBuffers()
    {
        strncpy(gameTitleBuf, gameTitle.c_str(), sizeof(gameTitleBuf) - 1);
        gameTitleBuf[sizeof(gameTitleBuf) - 1] = '\0';
        strncpy(targetNameBuf, targetName.c_str(), sizeof(targetNameBuf) - 1);
        targetNameBuf[sizeof(targetNameBuf) - 1] = '\0';
    }

    void syncFromBuffers()
    {
        gameTitle = gameTitleBuf;
        targetName = targetNameBuf;
    }
};
