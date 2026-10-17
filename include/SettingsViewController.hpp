#pragma once

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

DECLARE_CLASS_CODEGEN(AudioLink, SettingsViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD_PRIVATE(TMPro::TextMeshProUGUI*, _testPlaneLabel);
    DECLARE_INSTANCE_FIELD_PRIVATE(TMPro::TextMeshProUGUI*, _gainLabel);
    DECLARE_INSTANCE_FIELD_PRIVATE(TMPro::TextMeshProUGUI*, _bassLabel);
    DECLARE_INSTANCE_FIELD_PRIVATE(TMPro::TextMeshProUGUI*, _trebleLabel);
    DECLARE_INSTANCE_FIELD_PRIVATE(TMPro::TextMeshProUGUI*, _fadeLengthLabel);

    DECLARE_OVERRIDE_METHOD_MATCH(
        void,
        DidActivate,
        &::HMUI::ViewController::DidActivate,
        bool firstActivation,
        bool addedToHierarchy,
        bool screenSystemEnabling
    );
    DECLARE_INSTANCE_METHOD(void, RefreshLabels);

    public:
        DECLARE_DEFAULT_CTOR();
};
