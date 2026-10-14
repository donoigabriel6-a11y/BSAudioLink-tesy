#include "Providers/GameProvider.hpp"
#include "Logger.hpp"

DEFINE_TYPE(AudioLink, GameProvider);

extern Logger& getLogger();
namespace AudioLink {

    void GameProvider::ctor(AudioLinkComponent* audioLink, GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::ColorScheme* colorScheme) {
        static auto _audioLink_info = il2cpp_functions::class_get_field_from_name(klass, "_audioLink");
        static auto _audioTimeSyncController_info = il2cpp_functions::class_get_field_from_name(klass, "_audioTimeSyncController");
        static auto _colorScheme_info = il2cpp_functions::class_get_field_from_name(klass, "_colorScheme");

        il2cpp_functions::field_set_value_object(this, _audioLink_info, audioLink);
        il2cpp_functions::field_set_value_object(this, _audioTimeSyncController_info, audioTimeSyncController);
        il2cpp_functions::field_set_value_object(this, _colorScheme_info, colorScheme);
        _audioSourceAssigned = false;
        _colorSchemeAssigned = false;
    }

    void GameProvider::Initialize() {
        getLogger().info("GameProvider Initialize");
        // AudioTimeSyncController can exist before its AudioSource is ready on Quest.
        // Tick retries until the source and color scheme are available.
        Tick();
    }

    void GameProvider::Tick() {
        // AudioLinkComponent and ColorScheme are generated IL2CPP classes but do
        // not expose Unity's m_CachedPtr field in their generated C++ wrappers.
        if (!_audioLink) return;

        if (!_audioSourceAssigned) {
            auto controller = _audioTimeSyncController;
            // AudioTimeSyncController stores its source as UnityW<AudioSource>.
            // Retry until the controller and its source have both been initialized.
            if (controller && controller->m_CachedPtr.m_value && controller->_audioSource) {
                _audioLink->SetAudioSource(controller->_audioSource);
                _audioSourceAssigned = true;
                getLogger().info("GameProvider: gameplay audio source assigned");
            }
        }

        if (!_colorSchemeAssigned && _colorScheme) {
            _audioLink->SetColorScheme(_colorScheme);
            _colorSchemeAssigned = true;
            getLogger().info("GameProvider: gameplay color scheme assigned");
        }
    }
}