#include "Providers/MenuProvider.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "paper/shared/logger.hpp"

#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
DEFINE_TYPE(AudioLink, MenuProvider);

extern Logger& getLogger();

namespace AudioLink {
    MenuProvider* MenuProvider::instance;
    MenuProvider* MenuProvider::get_instance() {
        return instance;
    }

    void MenuProvider::ctor(AudioLinkComponent* audioLink, GlobalNamespace::PlayerDataModel* playerDataModel) {
        getLogger().info("MenuProvider ctor!");
        static auto _audioLink_info = il2cpp_functions::class_get_field_from_name(klass, "_audioLink");
        static auto _playerDataModel_info = il2cpp_functions::class_get_field_from_name(klass, "_playerDataModel");

        il2cpp_functions::field_set_value_object(this, _audioLink_info, audioLink);
        il2cpp_functions::field_set_value_object(this, _playerDataModel_info, playerDataModel);
        instance = this;
    }

    void MenuProvider::dtor() {
        instance = nullptr;
    }

    void MenuProvider::SongPreviewPlayerProvide(int activeChannel, ArrayW<GlobalNamespace::SongPreviewPlayer::AudioSourceVolumeController*> audioSourceControllers) {
        if (!_audioLink) {
            getLogger().error("MenuProvider: AudioLink component is unavailable");
            return;
        }
        if (!audioSourceControllers || activeChannel < 0 || activeChannel >= audioSourceControllers.size()) {
            getLogger().error("MenuProvider: invalid active audio channel %d", activeChannel);
            return;
        }
        auto controller = audioSourceControllers[activeChannel];
        if (!controller || !controller->audioSource ||
            !controller->audioSource->m_CachedPtr.m_value) {
            getLogger().error("MenuProvider: active audio source controller is unavailable");
            return;
        }
        _audioLink->SetAudioSource(controller->audioSource);
    }

    void MenuProvider::ColorManagerInstallerProvide(GlobalNamespace::ColorSchemeSO* menuColorScheme) {
        if (!_audioLink) {
            getLogger().error("MenuProvider: AudioLink component is unavailable for color update");
            return;
        }
        if (!menuColorScheme || !menuColorScheme->m_CachedPtr.m_value) {
            getLogger().error("MenuProvider: menu color scheme is unavailable");
            return;
        }
        auto playerData = _playerDataModel ? _playerDataModel->playerData : nullptr;
        auto colorSettings = playerData ? playerData->colorSchemesSettings : nullptr;
        auto overrideColorScheme = colorSettings ? colorSettings->GetOverrideColorScheme() : nullptr;
        auto chosenScheme = overrideColorScheme ? overrideColorScheme : menuColorScheme->get_colorScheme();
        if (chosenScheme) _audioLink->SetColorScheme(chosenScheme);
    }
}