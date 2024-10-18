// Standard Library
#include <any>
#include <string>
#include <unordered_map>

// Hyprland Dependencies
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/helpers/Color.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/SharedDefs.hpp>
#include <hyprland/src/config/ConfigValue.hpp>
#include <hyprlang.hpp>

// Project
#include "globals.hpp"

using EventMap = std::unordered_map<std::string, std::any>;

struct ZoomState {
    double zoom = 1.0;
    double default_delta = 0.05;
    double last_delta = 0.0;
    double factor = 10.0;
    bool enabled = false;
    bool reset = false;
};

static SP<HOOK_CALLBACK_FN> MOUSE_AXIS_CALLBACK = nullptr;
static ZoomState ZOOM_STATE {};

[[maybe_unused]] static constexpr int DEFAULT_SCROLL_DELTA = 15;

static void hookMouseAxis([[maybe_unused]] IPointer::SAxisEvent event) {
    if (!ZOOM_STATE.enabled) {
        ZOOM_STATE.enabled = false;
        ZOOM_STATE.zoom = 1.0;
        ZOOM_STATE.last_delta = 0.0;
        HyprlandAPI::invokeHyprctlCommand("keyword", std::format("cursor:zoom_factor {:.2}", 1.0));
        (*(OriginalMouseAxis)MOUSE_AXIS_HOOK->m_pOriginal)(event);
        return;
    }

    // Yoinked from hyprland/src/managers/input/InputManager.cpp:CInputManager::onMouseWheel()
    static auto PINPUTSCROLLFACTOR = CConfigValue<Hyprlang::FLOAT>("input:scroll_factor");
    static auto PTOUCHPADSCROLLFACTOR = CConfigValue<Hyprlang::FLOAT>("input:touchpad:scroll_factor");
    auto factor = (*PTOUCHPADSCROLLFACTOR <= 0.f || event.source == WL_POINTER_AXIS_SOURCE_FINGER ? *PTOUCHPADSCROLLFACTOR : *PINPUTSCROLLFACTOR);
    double delta = event.delta * factor;
    const auto invert = *static_cast<bool*>(*HyprlandAPI::getConfigValue(HYPR_HANDLE, "plugin:hyprsquint:invert_scroll")->getDataStaticPtr());
    const auto squintDelta = delta/DEFAULT_SCROLL_DELTA; 
    ZOOM_STATE.zoom = std::clamp(ZOOM_STATE.zoom - ((invert) ? -squintDelta : squintDelta), 1.0, 24.0);
    ZOOM_STATE.last_delta = delta;
    HyprlandAPI::invokeHyprctlCommand("keyword", std::format("cursor:zoom_factor {:.2}", ZOOM_STATE.zoom));
}

static void onSquintHandler(std::string arg) {
    if (arg == "enable") {
        ZOOM_STATE.enabled = true;
    } else if (arg == "disable") {
        ZOOM_STATE.enabled = false;
        ZOOM_STATE.zoom = 1.0;
        ZOOM_STATE.last_delta = 0.0;
        HyprlandAPI::invokeHyprctlCommand("keyword", std::format("cursor:zoom_factor {:.2}", 1.0));
    } else {
        ZOOM_STATE.enabled = !ZOOM_STATE.enabled;
    }
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    HYPR_HANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();
    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(HYPR_HANDLE, "[hyprsquint] Mismatched headers!",
                                     CColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprsquint] Version mismatch");
    }

    static auto const METHODS = HyprlandAPI::findFunctionsByName(HYPR_HANDLE, "onMouseWheel");
    MOUSE_AXIS_HOOK = HyprlandAPI::createFunctionHook(HYPR_HANDLE, METHODS[0].address, reinterpret_cast<void*>(&hookMouseAxis));
    if (!MOUSE_AXIS_HOOK) {
        HyprlandAPI::addNotification(HYPR_HANDLE, "[hyprsquint] Could not bind mouseAxis hook!",
                                     CColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprsquint] Could not bind mouseAxis hook!");
    }
    MOUSE_AXIS_HOOK->hook();

    HyprlandAPI::addDispatcher(HYPR_HANDLE, "hyprsquint:squint", onSquintHandler);

    HyprlandAPI::addConfigValue(HYPR_HANDLE, "plugin:hyprsquint:invert_scroll", Hyprlang::INT{0});
    HyprlandAPI::reloadConfig();

    return {
        "hyprsquint",
        "A Hyprland plugin for magnifying your screen.",
        "slothsh",
        "0.0.3",
    };
}

APICALL EXPORT void PLUGIN_EXIT() {
    if (MOUSE_AXIS_CALLBACK != nullptr) { MOUSE_AXIS_CALLBACK = nullptr; }
}
