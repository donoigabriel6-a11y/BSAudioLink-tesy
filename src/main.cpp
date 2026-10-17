#include "beatsaber-hook/shared/utils/utils.h"
#include "Logger.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "scotland2/shared/modloader.h"
#include <dlfcn.h>
#include <android/log.h>
#include "custom-types/shared/register.hpp"

#if __has_include("pinkcore/shared/RequirementAPI.hpp")
#include "pinkcore/shared/RequirementAPI.hpp"
#define PINKCORE
#endif

#include "config.hpp"
#include "AssetBundleManager.hpp"
#include "AudioLink.hpp"
#include "Providers/GameProvider.hpp"
#include "Providers/MenuProvider.hpp"

#include "Zenject/DiContainer.hpp"
#include "Zenject/FromBinderNonGeneric.hpp"
#include "Zenject/ConcreteIdBinderGeneric_1.hpp"
#include "GlobalNamespace/StandardGameplayInstaller.hpp"
#include "GlobalNamespace/MissionGameplayInstaller.hpp"
#include "GlobalNamespace/MultiplayerLocalActivePlayerInstaller.hpp"
#include "GlobalNamespace/MainSettingsMenuViewControllersInstaller.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/PCAppInit.hpp"
#include "GlobalNamespace/QuestAppInit.hpp"
#include "GlobalNamespace/ColorManagerInstaller.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "GlobalNamespace/MainSettingsMenuViewController.hpp"
#include "GlobalNamespace/SettingsSubMenuInfo.hpp"

#include "lapiz/shared/zenject/Zenjector.hpp"
#include "SettingsViewController.hpp"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"

modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
Logger& getLogger();

namespace {
    constexpr const char* kLogTag = "BSAudioLink";
    constexpr const char* kAudioLinkCapability = "AudioLink";
    constexpr __SIZE_TYPE__ kAudioLinkCapabilityLength = 9;

    using RegisterCapability_t = void (*)(const char*, __SIZE_TYPE__);
    static bool g_songCoreCapabilityRegistered = false;

    void CapabilityLogInfo(const char* message) {
        __android_log_print(ANDROID_LOG_INFO, kLogTag, "%s", message);
    }

    void CapabilityLogError(const char* message) {
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "%s", message);
    }

    bool RegisterSongCoreCapability() {
        if (g_songCoreCapabilityRegistered) return true;

        // Verified SongCore Quest ABI: RegisterCapability(std::string_view) is passed
        // as pointer + length on ARM64. Keep both NDK and libc++ symbol variants.
        static constexpr const char* kNdkSymbol =
            "_ZN8SongCore3API12Capabilities18RegisterCapabilityENSt6__ndk117basic_string_viewIcNS2_11char_traitsIcEEEE";
        static constexpr const char* kStdSymbol =
            "_ZN8SongCore3API12Capabilities18RegisterCapabilityESt17basic_string_viewIcSt11char_traitsIcEE";

        auto fn = reinterpret_cast<RegisterCapability_t>(dlsym(RTLD_DEFAULT, kNdkSymbol));
        if (!fn) fn = reinterpret_cast<RegisterCapability_t>(dlsym(RTLD_DEFAULT, kStdSymbol));

        if (!fn) {
            CapabilityLogInfo("SongCore capability symbol not in global scope; trying libsongcore.so");
            void* handle = dlopen("libsongcore.so", RTLD_NOW | RTLD_GLOBAL);
            if (handle) {
                fn = reinterpret_cast<RegisterCapability_t>(dlsym(handle, kNdkSymbol));
                if (!fn) fn = reinterpret_cast<RegisterCapability_t>(dlsym(handle, kStdSymbol));
                // Keep the handle open: the resolved function belongs to this library.
            } else {
                const char* error = dlerror();
                __android_log_print(ANDROID_LOG_ERROR, kLogTag,
                                    "dlopen libsongcore.so failed: %s", error ? error : "unknown error");
            }
        }

        if (!fn) {
            CapabilityLogError("SongCore RegisterCapability symbol not found; will retry at late_load");
            return false;
        }

        fn(kAudioLinkCapability, kAudioLinkCapabilityLength);
        g_songCoreCapabilityRegistered = true;
        CapabilityLogInfo("Registered SongCore capability: AudioLink");
        return true;
    }

    bool RegisterSettingsSubMenu(GlobalNamespace::MainSettingsMenuViewController* menu) {
        if (!menu) {
            CapabilityLogError("Beat Saber activated a null settings menu");
            getLogger().error("AudioLink settings: Beat Saber activated a null settings menu");
            return false;
        }
        auto existing = menu->_settingsSubMenuInfos;
        const int32_t existingCount = existing ? existing.size() : 0;
        for (int32_t i = 0; i < existingCount; ++i) {
            auto info = existing[i];
            if (info && info->____menuName == "AudioLink") {
                getLogger().info("AudioLink settings submenu already exists");
                return true;
            }
        }

        auto expanded = ArrayW<GlobalNamespace::SettingsSubMenuInfo*>(
            Array<GlobalNamespace::SettingsSubMenuInfo*>::NewLength(existingCount + 1)
        );
        for (int32_t i = 0; i < existingCount; ++i) expanded[i] = existing[i];

        auto info = GlobalNamespace::SettingsSubMenuInfo::New_ctor();
        auto settingsView = AudioLink::SettingsViewController::New_ctor();
        if (!info || !settingsView) {
            CapabilityLogError("Could not create the AudioLink settings page");
            getLogger().error("AudioLink settings: could not create the submenu or view controller");
            return false;
        }
        info->__cordl_internal_set__menuName("AudioLink");
        info->__cordl_internal_set__viewController(settingsView);
        expanded[existingCount] = info;
        menu->__cordl_internal_set__settingsSubMenuInfos(expanded);
        CapabilityLogInfo("Added AudioLink to Beat Saber settings");
        getLogger().info("AudioLink settings submenu registered (%d existing entries)", existingCount);
        return true;
    }
}

Logger& getLogger() {
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

MAKE_HOOK_MATCH(SongPreviewPlayer_CrossFadeTo, static_cast<void (GlobalNamespace::SongPreviewPlayer::*)(::UnityEngine::AudioClip*, float, float, float, bool, ::System::Action*)>(&GlobalNamespace::SongPreviewPlayer::CrossfadeTo), void, GlobalNamespace::SongPreviewPlayer* self, ::UnityEngine::AudioClip* audioClip, float musicVolume, float startTime, float duration, bool isDefault, ::System::Action* onFadeOutCallback) {
    getLogger().info("SongPreviewPlayer_CrossFadeTo");
    SongPreviewPlayer_CrossFadeTo(self, audioClip, musicVolume, startTime, duration, isDefault, onFadeOutCallback);
    auto menuProvider = AudioLink::MenuProvider::get_instance();
    if (menuProvider) {
        menuProvider->SongPreviewPlayerProvide(self->_activeChannel, self->_audioSourceControllers);
    } else {
        getLogger().info("No menu provider exists!");
    }
}

MAKE_HOOK_MATCH(ColorManagerInstaller_InstallBindings, &GlobalNamespace::ColorManagerInstaller::InstallBindings, void, GlobalNamespace::ColorManagerInstaller* self) {
    getLogger().info("ColorManagerInstaller_InstallBindings");
    ColorManagerInstaller_InstallBindings(self);
    auto menuProvider = AudioLink::MenuProvider::get_instance();
    if (menuProvider) {
        menuProvider->ColorManagerInstallerProvide(self->_menuColorScheme);
    } else {
        getLogger().info("No menu provider exists!");
    }
}

MAKE_HOOK_MATCH(MainSettingsMenuViewController_DidActivate, &GlobalNamespace::MainSettingsMenuViewController::DidActivate, void, GlobalNamespace::MainSettingsMenuViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    getLogger().info("Main settings view activated; registering AudioLink settings");
    RegisterSettingsSubMenu(self);
    MainSettingsMenuViewController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
}

extern "C" __attribute__((visibility("default"))) void setup(CModInfo* info) noexcept {
    if (!info) return;
    info->id = MOD_ID;
    info->version = VERSION;
    info->version_long = 0;
}

extern "C" __attribute__((visibility("default"))) void load() {
    getLogger().info("AudioLink load() called");
    CapabilityLogInfo("AudioLink native library loaded");
    il2cpp_functions::Init();

    if (!LoadConfig()) SaveConfig();
    custom_types::Register::AutoRegister();

    auto& logger = getLogger();
    auto zenjector = Lapiz::Zenject::Zenjector::Get();
    zenjector->Install(Lapiz::Zenject::Location::Player, [](::Zenject::DiContainer* container){
        container->BindInterfacesTo<AudioLink::GameProvider*>()->AsSingle()->NonLazy();
    });
    zenjector->Install(Lapiz::Zenject::Location::App, [](::Zenject::DiContainer* container){
        container->BindInterfacesAndSelfTo<AudioLink::AssetBundleManager*>()->AsSingle();
        container->BindInterfacesAndSelfTo<AudioLink::AudioLinkComponent*>()->AsSingle();
    });
    zenjector->Install(Lapiz::Zenject::Location::Menu, [](::Zenject::DiContainer* container){
        container->Bind<AudioLink::MenuProvider*>()->AsSingle()->NonLazy();
    });

    INSTALL_HOOK(logger, SongPreviewPlayer_CrossFadeTo);
    INSTALL_HOOK(logger, ColorManagerInstaller_InstallBindings);
    INSTALL_HOOK(logger, MainSettingsMenuViewController_DidActivate);

#ifdef PINKCORE
    PinkCore::RequirementAPI::RegisterInstalled("AudioLink");
#endif

    // Try now in case SongCore is already loaded. If not, late_load retries after
    // dependencies have been loaded. The registration guard prevents duplicates.
    RegisterSongCoreCapability();
}

extern "C" __attribute__((visibility("default"))) void late_load() noexcept {
    CapabilityLogInfo("AudioLink late_load called; registering SongCore capability");
    RegisterSongCoreCapability();
}
