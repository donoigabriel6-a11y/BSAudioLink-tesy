#pragma once

struct config_t {
    bool showTestPlane = true;
};

void SaveConfig();
bool LoadConfig();

extern config_t config;