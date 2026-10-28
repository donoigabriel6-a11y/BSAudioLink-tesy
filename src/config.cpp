#include "config.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "Logger.hpp"
#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/config/config-utils.hpp"

#include <algorithm>
#include <cmath>

config_t config;

extern Logger& getLogger();
extern modloader::ModInfo modInfo;
Configuration& get_config() {
    static Configuration config(modInfo);
    static bool loaded = false;
    if (!loaded) {
        config.Load();
        loaded = true;
    }
    return config;
}

#define Save(identifier) doc.AddMember(#identifier, config.identifier, allocator)

void SaveConfig() {
    getLogger().info("Saving Configuration...");
    rapidjson::Document& doc = get_config().config;

    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    Save(showTestPlane);
    Save(gain);
    Save(bass);
    Save(treble);
    Save(fadeLength);
    Save(autogain);
    Save(autogainDerate);
    get_config().Write();
    getLogger().info("Saved Configuration!");
}

#define GET_BOOL(identifier)                                            \
    auto identifier##_itr = doc.FindMember(#identifier);                \
    if (identifier##_itr != doc.MemberEnd() && identifier##_itr->value.IsBool()) \
        config.identifier = identifier##_itr->value.GetBool();          \
    else                                                                \
        foundEverything = false;

#define GET_FLOAT(identifier)                                           \
    auto identifier##_itr = doc.FindMember(#identifier);                 \
    if (identifier##_itr != doc.MemberEnd() && identifier##_itr->value.IsNumber()) \
        config.identifier = identifier##_itr->value.GetFloat();        \
    else                                                                \
        foundEverything = false;

bool LoadConfig() {
    getLogger().info("Loading Configuration...");
    bool foundEverything = true;
    rapidjson::Document& doc = get_config().config;
    if (!doc.IsObject()) {
        getLogger().error("Configuration is not a JSON object; resetting to defaults");
        return false;
    }

    GET_BOOL(showTestPlane);
    GET_FLOAT(gain);
    GET_FLOAT(bass);
    GET_FLOAT(treble);
    GET_FLOAT(fadeLength);
    GET_BOOL(autogain);
    GET_FLOAT(autogainDerate);

    config.gain = std::isfinite(config.gain) ? std::clamp(config.gain, 0.0f, 2.0f) : 1.0f;
    config.bass = std::isfinite(config.bass) ? std::clamp(config.bass, 0.0f, 2.0f) : 1.0f;
    config.treble = std::isfinite(config.treble) ? std::clamp(config.treble, 0.0f, 2.0f) : 1.0f;
    config.fadeLength = std::isfinite(config.fadeLength) ? std::clamp(config.fadeLength, 0.0f, 1.0f) : 0.25f;
    config.autogainDerate = std::isfinite(config.autogainDerate) ? std::clamp(config.autogainDerate, 0.001f, 1.0f) : 0.1f;

    if (foundEverything)
        getLogger().info("Loaded Configuration!");
    return foundEverything;
}