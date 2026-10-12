#include "Providers/GameProvider.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"

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
    }

    void GameProvider::Initialize() {
        getLogger().info("GameProvider Initialize");
        if (!_audioLink) {
            getLogger().error("GameProvider: AudioLink component is unavailable");
            return;
        }
        if (_audioTimeSyncController && _audioTimeSyncController->m_CachedPtr.m_value &&
            _audioTimeSyncController->_audioSource && _audioTimeSyncController->_audioSource->m_CachedPtr.m_value) {
            _audioLink->SetAudioSource(_audioTimeSyncController->_audioSource);
        } else {
            getLogger().error("GameProvider: gameplay audio source is not available yet");
        }
        if (_colorScheme) {
            _audioLink->SetColorScheme(_colorScheme);
        } else {
            getLogger().error("GameProvider: gameplay color scheme is not available");
        }
    }
}