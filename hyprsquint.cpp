// Standard Library
#include <any>
#include <string>
#include <unordered_map>

// Hyprland Dependencies
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/helpers/Color.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/SharedDefs.hpp>

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

static void handleMouseAxisEvent([[maybe_unused]] void* handle, [[maybe_unused]] SCallbackInfo& callback_info, [[maybe_unused]] std::any args) {
    const auto user_args = std::any_cast<EventMap>(args);

    if (!ZOOM_STATE.enabled) {
        ZOOM_STATE.enabled = false;
        ZOOM_STATE.zoom = 1.0;
        ZOOM_STATE.last_delta = 0.0;
        HyprlandAPI::invokeHyprctlCommand("keyword", std::format("cursor:zoom_factor {:.2}", 1.0));
        return;
    }

    if (user_args.contains("event")) {
        const auto scroll = std::any_cast<IPointer::SAxisEvent>(user_args.at("event"));
        ZOOM_STATE.last_delta = scroll.delta;
        ZOOM_STATE.zoom = std::clamp(ZOOM_STATE.zoom - scroll.delta/DEFAULT_SCROLL_DELTA/2.0, 1.0, 24.0);
        ZOOM_STATE.last_delta = scroll.delta;
        HyprlandAPI::invokeHyprctlCommand("keyword", std::format("cursor:zoom_factor {:.2}", ZOOM_STATE.zoom));
    }
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

    MOUSE_AXIS_CALLBACK = HyprlandAPI::registerCallbackDynamic(HYPR_HANDLE, "mouseAxis", &handleMouseAxisEvent);

    HyprlandAPI::addDispatcher(HYPR_HANDLE, "hyprsquint:squint", onSquintHandler);

    return {
        "hyprsquint",
        "A Hyprland plugin for magnifying your screen.",
        "slothsh",
        "0.0.1",
    };
}

APICALL EXPORT void PLUGIN_EXIT() {
    if (MOUSE_AXIS_CALLBACK != nullptr) { MOUSE_AXIS_CALLBACK = nullptr; }
}
