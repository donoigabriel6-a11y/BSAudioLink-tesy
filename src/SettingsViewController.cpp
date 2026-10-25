#include "SettingsViewController.hpp"

#include "AudioLink.hpp"
#include "Logger.hpp"
#include "config.hpp"

#include "TMPro/TMP_FontAsset.hpp"
#include "TMPro/TMP_Settings.hpp"
#include "UnityEngine/CanvasRenderer.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "System/Type.hpp"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"
#include "custom-types/shared/delegate.hpp"

#include <cstdio>
#include <initializer_list>

DEFINE_TYPE(AudioLink, SettingsViewController);

extern Logger& getLogger();

namespace {
    using ToggleCallback = std::function<void()>;

    TMPro::TextMeshProUGUI* CreateLabel(UnityEngine::Transform* parent, float y) {
        auto rectTransformType = System::Type::GetType("UnityEngine.RectTransform, UnityEngine.CoreModule");
        auto canvasRendererType = System::Type::GetType("UnityEngine.CanvasRenderer, UnityEngine.CoreModule");
        auto textType = System::Type::GetType("TMPro.TextMeshProUGUI, Unity.TextMeshPro");
        if (!rectTransformType || !canvasRendererType || !textType) {
            getLogger().error("AudioLink settings: required Unity UI text type was not found");
            return nullptr;
        }
        auto components = ArrayW<System::Type*>({
            rectTransformType,
            canvasRendererType,
            textType
        });
        auto labelObject = UnityEngine::GameObject::New_ctor("AudioLink Setting Label", components);
        if (!labelObject || !labelObject->m_CachedPtr.m_value) {
            getLogger().error("AudioLink settings: could not create a text label");
            return nullptr;
        }

        auto labelTransform = labelObject->GetComponent<UnityEngine::RectTransform*>();
        if (!labelTransform || !labelTransform->m_CachedPtr.m_value) {
            getLogger().error("AudioLink settings: label has no RectTransform");
            return nullptr;
        }
        labelTransform->SetParent(parent, false);
        labelTransform->set_anchoredPosition({0.0f, y});
        labelTransform->set_sizeDelta({100.0f, 12.0f});

        auto label = labelObject->GetComponent<TMPro::TextMeshProUGUI*>();
        if (!label || !label->m_CachedPtr.m_value) {
            getLogger().error("AudioLink settings: label has no TextMeshProUGUI component");
            return nullptr;
        }
        auto font = TMPro::TMP_Settings::get_defaultFontAsset();
        if (font && font->m_CachedPtr.m_value) label->set_font(font);
        label->set_fontSize(5.0f);
        label->set_color(UnityEngine::Color(1.0f, 1.0f, 1.0f, 1.0f));
        label->set_alignment(TMPro::TextAlignmentOptions::Center);
        label->set_raycastTarget(false);
        return label;
    }

    TMPro::TextMeshProUGUI* CreateButton(
        UnityEngine::Transform* parent,
        float y,
        const char* initialText,
        ToggleCallback callback
    ) {
        auto rectTransformType = System::Type::GetType("UnityEngine.RectTransform, UnityEngine.CoreModule");
        auto canvasRendererType = System::Type::GetType("UnityEngine.CanvasRenderer, UnityEngine.CoreModule");
        auto imageType = System::Type::GetType("UnityEngine.UI.Image, UnityEngine.UI");
        auto buttonType = System::Type::GetType("UnityEngine.UI.Button, UnityEngine.UI");
        if (!rectTransformType || !canvasRendererType || !imageType || !buttonType) {
            getLogger().error("AudioLink settings: required Unity UI button type was not found");
            return nullptr;
        }
        auto components = ArrayW<System::Type*>({
            rectTransformType,
            canvasRendererType,
            imageType,
            buttonType
        });
        auto buttonObject = UnityEngine::GameObject::New_ctor("AudioLink Setting Button", components);
        if (!buttonObject || !buttonObject->m_CachedPtr.m_value) {
            getLogger().error("AudioLink settings: could not create a button object");
            return nullptr;
        }

        auto buttonTransform = buttonObject->GetComponent<UnityEngine::RectTransform*>();
        if (!buttonTransform || !buttonTransform->m_CachedPtr.m_value) {
            getLogger().error("AudioLink settings: button has no RectTransform");
            return nullptr;
        }
        buttonTransform->SetParent(parent, false);
        buttonTransform->set_anchoredPosition({0.0f, y});
        buttonTransform->set_sizeDelta({100.0f, 14.0f});

        auto image = buttonObject->GetComponent<UnityEngine::UI::Image*>();
        if (image && image->m_CachedPtr.m_value) {
            image->set_color(UnityEngine::Color(0.12f, 0.15f, 0.2f, 0.95f));
            image->set_raycastTarget(true);
        }

        auto button = buttonObject->GetComponent<UnityEngine::UI::Button*>();
        if (!button || !button->m_CachedPtr.m_value || !image || !image->m_CachedPtr.m_value) {
            getLogger().error("AudioLink settings: could not create an interactive button");
            return nullptr;
        }
        button->set_targetGraphic(image);
        button->get_onClick()->AddListener(custom_types::MakeDelegate<UnityEngine::Events::UnityAction*>(callback));

        auto label = CreateLabel(buttonTransform, 0.0f);
        if (label) label->set_text(initialText);
        return label;
    }

    float Cycle(float value, std::initializer_list<float> values) {
        for (auto it = values.begin(); it != values.end(); ++it) {
            if (value == *it) {
                ++it;
                return it == values.end() ? *values.begin() : *it;
            }
        }
        return *values.begin();
    }
}

namespace AudioLink {
    void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        HMUI::ViewController::DidActivate(firstActivation, addedToHierarchy, screenSystemEnabling);
        getLogger().info("AudioLink settings page activated (first activation: %s)", firstActivation ? "yes" : "no");
        if (!firstActivation) return;

        auto root = get_transform();
        if (!root || !root->m_CachedPtr.m_value) {
            getLogger().error("AudioLink settings: settings view has no transform");
            return;
        }

        auto heading = CreateLabel(root, 50.0f);
        if (heading) {
            heading->set_text("AudioLink Settings");
            heading->set_fontSize(6.0f);
        }
        _testPlaneLabel = CreateButton(root, 28.0f, "", [this] {
            config.showTestPlane = !config.showTestPlane;
            SaveConfig();
            if (auto component = AudioLinkComponent::get_instance()) component->RefreshSettings();
            RefreshLabels();
        });
        _gainLabel = CreateButton(root, 8.0f, "", [this] {
            config.gain = Cycle(config.gain, {0.0f, 0.5f, 1.0f, 1.5f, 2.0f});
            SaveConfig();
            if (auto component = AudioLinkComponent::get_instance()) component->RefreshSettings();
            RefreshLabels();
        });
        _bassLabel = CreateButton(root, -12.0f, "", [this] {
            config.bass = Cycle(config.bass, {0.0f, 0.5f, 1.0f, 1.5f, 2.0f});
            SaveConfig();
            if (auto component = AudioLinkComponent::get_instance()) component->RefreshSettings();
            RefreshLabels();
        });
        _trebleLabel = CreateButton(root, -32.0f, "", [this] {
            config.treble = Cycle(config.treble, {0.0f, 0.5f, 1.0f, 1.5f, 2.0f});
            SaveConfig();
            if (auto component = AudioLinkComponent::get_instance()) component->RefreshSettings();
            RefreshLabels();
        });
        _fadeLengthLabel = CreateButton(root, -52.0f, "", [this] {
            config.fadeLength = Cycle(config.fadeLength, {0.0f, 0.25f, 0.5f, 0.75f, 1.0f});
            SaveConfig();
            if (auto component = AudioLinkComponent::get_instance()) component->RefreshSettings();
            RefreshLabels();
        });
        RefreshLabels();
    }

    void SettingsViewController::RefreshLabels() {
        char text[64]{};
        if (_testPlaneLabel && _testPlaneLabel->m_CachedPtr.m_value) {
            std::snprintf(text, sizeof(text), "Test plane: %s", config.showTestPlane ? "On" : "Off");
            _testPlaneLabel->set_text(text);
        }
        if (_gainLabel && _gainLabel->m_CachedPtr.m_value) {
            std::snprintf(text, sizeof(text), "Gain: %.2f", config.gain);
            _gainLabel->set_text(text);
        }
        if (_bassLabel && _bassLabel->m_CachedPtr.m_value) {
            std::snprintf(text, sizeof(text), "Bass: %.2f", config.bass);
            _bassLabel->set_text(text);
        }
        if (_trebleLabel && _trebleLabel->m_CachedPtr.m_value) {
            std::snprintf(text, sizeof(text), "Treble: %.2f", config.treble);
            _trebleLabel->set_text(text);
        }
        if (_fadeLengthLabel && _fadeLengthLabel->m_CachedPtr.m_value) {
            std::snprintf(text, sizeof(text), "Fade length: %.2f", config.fadeLength);
            _fadeLengthLabel->set_text(text);
        }
    }
}
