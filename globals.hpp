#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>

inline HANDLE PHANDLE = nullptr;

struct SGlobalState {
  CShader borderShader1;
  CShader borderShader0;
  wl_event_source* tick = nullptr;
};

inline std::unique_ptr<SGlobalState> g_pGlobalState;
