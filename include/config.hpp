#pragma once

struct config_t {
    bool showTestPlane = false;
    float gain = 1.0f;
    float bass = 1.0f;
    float treble = 1.0f;
    float fadeLength = 0.25f;
};

void SaveConfig();
bool LoadConfig();

extern config_t config;