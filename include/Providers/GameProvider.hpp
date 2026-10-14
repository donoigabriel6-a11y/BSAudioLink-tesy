#pragma once

#include "custom-types/shared/macros.hpp"
#include "lapiz/shared/macros.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "Zenject/IInitializable.hpp"
#include "Zenject/ITickable.hpp"

#include "AudioLink.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(AudioLink, GameProvider, Il2CppObject, Zenject::IInitializable*, Zenject::ITickable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(::AudioLink::AudioLinkComponent*, _audioLink);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::AudioTimeSyncController*, _audioTimeSyncController);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::ColorScheme*, _colorScheme);
    DECLARE_INSTANCE_FIELD_PRIVATE(bool, _audioSourceAssigned);
    DECLARE_INSTANCE_FIELD_PRIVATE(bool, _colorSchemeAssigned);

    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Tick, &::Zenject::ITickable::Tick);
    public:
        DECLARE_CTOR(ctor, ::AudioLink::AudioLinkComponent* audioLink, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::ColorScheme* colorScheme);
};
