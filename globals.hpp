#pragma once

#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/plugins/HookSystem.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/SharedDefs.hpp>

// Declare global handle pointer
inline HANDLE HYPR_HANDLE = nullptr;
inline CFunctionHook* MOUSE_AXIS_HOOK = nullptr;
using OriginalMouseAxis = void(*)(IPointer::SAxisEvent);
