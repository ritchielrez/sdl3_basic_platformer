#pragma once
// Debug overlay powered by Dear ImGui. Only compiled in debug builds — this
// file is entirely excluded from release binaries. ImGui is a bloat-free C++
// GUI library widely used in game development for in-game debugging tools,
// level editors, and profilers.
#ifdef DEBUG

#include <backends/imgui_impl_sdl3.h>
#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

#include "Player.h"
#include "SDLState.h"
#include "Slime.h"

// Renders an inspectable overlay showing player state, enemy info, camera
// position, and frame timing. Activated by pressing F1 during gameplay.
// Uses ImGui which integrates directly with SDL3 and SDL_Renderer.
struct DebugUI {
  const SDLState &sdlState;
  float fontHeight;

  DebugUI() = delete;

  DebugUI(const SDLState &sdlState, const std::string_view &fontPath)
      : sdlState(sdlState) {
    fontHeight = 18.0f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(sdlState.win, sdlState.renderer);
    ImGui_ImplSDLRenderer3_Init(sdlState.renderer);

    io.Fonts->AddFontFromFileTTF(fontPath.data(), fontHeight);
  }
  DebugUI(const SDLState &sdlState, const std::string_view &fontPath,
          const float fontHeight)
      : sdlState(sdlState), fontHeight(fontHeight) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(sdlState.win, sdlState.renderer);
    ImGui_ImplSDLRenderer3_Init(sdlState.renderer);

    io.Fonts->AddFontFromFileTTF(fontPath.data(), fontHeight);
  }

  // Sub-panels: each inspects a specific game subsystem.
  void drawPlayerInfo(const Player &player);
  void drawCameraInfo(const SDL_FRect &cam);
  void drawSlimesInfo(const std::vector<Slime> &slimes);

  // ImGui lifecycle — called once per frame between SDL_RenderClear and
  // SDL_RenderPresent.
  void newFrame();
  void drawFrame(const Player &player, const std::vector<Slime> &slimes,
                 const SDL_FRect &cam);
  void presentFrame() const;

  ~DebugUI() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }
};
#endif
