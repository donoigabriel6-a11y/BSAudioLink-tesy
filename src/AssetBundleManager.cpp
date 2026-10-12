#include "AssetBundleManager.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"

#include "UnityEngine/AssetBundleRequest.hpp"
#include "UnityEngine/AssetBundleCreateRequest.hpp"

#include "assets.hpp"

DEFINE_TYPE(AudioLink, AssetBundleManager);

extern Logger& getLogger();

namespace AudioLink {
    UnityEngine::Material* AssetBundleManager::get_material() {
        return _material;
    }

    UnityEngine::RenderTexture* AssetBundleManager::get_renderTexture() {
        return _renderTexture;
    }

    void AssetBundleManager::Load() {
        if (_bundle && _bundle->m_CachedPtr.m_value) {
            if (_material && _material->m_CachedPtr.m_value &&
                _renderTexture && _renderTexture->m_CachedPtr.m_value) {
                return;
            }
        }

        using AssetBundle_LoadFromMemory = function_ptr_t<UnityEngine::AssetBundle*, ArrayW<uint8_t>, int>;
        static auto assetBundle_LoadFromMemory = reinterpret_cast<AssetBundle_LoadFromMemory>(
            il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemory_Internal"));

        if (!assetBundle_LoadFromMemory) {
            getLogger().error("AudioLink: AssetBundle.LoadFromMemory_Internal icall was not found");
            return;
        }

        static auto _bundle_info = il2cpp_functions::class_get_field_from_name(klass, "_bundle");
        static auto _material_info = il2cpp_functions::class_get_field_from_name(klass, "_material");
        static auto _renderTexture_info = il2cpp_functions::class_get_field_from_name(klass, "_renderTexture");

        auto bundle = assetBundle_LoadFromMemory(IncludedAssets::Bundle, 0);
        if (!bundle || !bundle->m_CachedPtr.m_value) {
            getLogger().error("AudioLink: embedded asset bundle failed to load");
            return;
        }
        il2cpp_functions::field_set_value_object(this, _bundle_info, bundle);
        getLogger().info("AudioLink: embedded asset bundle loaded");

        auto material = bundle->LoadAsset<UnityEngine::Material*>("assets/audiolink/materials/mat_audiolink.mat");
        if (!material || !material->m_CachedPtr.m_value) {
            getLogger().error("AudioLink: mat_audiolink.mat was not found in the asset bundle");
        } else {
            il2cpp_functions::field_set_value_object(this, _material_info, material);
            getLogger().info("AudioLink: AudioLink material loaded");
        }

        auto rt = bundle->LoadAsset<UnityEngine::RenderTexture*>("assets/audiolink/rendertextures/rt_audiolink.asset");
        if (!rt || !rt->m_CachedPtr.m_value) {
            getLogger().error("AudioLink: rt_audiolink.asset was not found in the asset bundle");
        } else {
            il2cpp_functions::field_set_value_object(this, _renderTexture_info, rt);
            getLogger().info("AudioLink: AudioLink render texture loaded");
        }
        getLogger().flush();
    }

    void AssetBundleManager::Dispose() {
        getLogger().info("AssetBundleManager Dispose");
        static auto _bundle_info = il2cpp_functions::class_get_field_from_name(klass, "_bundle");
        static auto _material_info = il2cpp_functions::class_get_field_from_name(klass, "_material");
        static auto _renderTexture_info = il2cpp_functions::class_get_field_from_name(klass, "_renderTexture");

        if (_bundle && _bundle->m_CachedPtr.m_value) {
            _bundle->Unload(true);
        }
        il2cpp_functions::field_set_value_object(this, _bundle_info, nullptr);

        if (_material && _material->m_CachedPtr.m_value) {
            UnityEngine::Object::DestroyImmediate(_material);
        }
        il2cpp_functions::field_set_value_object(this, _material_info, nullptr);

        if (_renderTexture && _renderTexture->m_CachedPtr.m_value) {
            UnityEngine::Object::DestroyImmediate(_renderTexture);
        }
        il2cpp_functions::field_set_value_object(this, _renderTexture_info, nullptr);
    }
}
