#pragma once

// Debug overlay rendered with Dear ImGui, a C++ immediate-mode GUI library
// commonly used for in-game tools.  Only compiled in debug builds — the entire
// struct is gated behind `#ifdef DEBUG` and stripped from release binaries.
// Press F1 during gameplay to show or hide the overlay.

#ifdef DEBUG

#include <backends/imgui_impl_sdl3.h>
#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

#include "Player.h"
#include "SDLState.h"
#include "Slime.h"

// Holds a reference to the shared SDL window and renderer so ImGui can draw on
// top of the game.  fontHeight controls the size of the UI text.  Two
// constructors are provided — one with a default 18px font, one that lets the
// caller pick a custom size.  The constructor initialises a fresh ImGui
// context, enables keyboard navigation and window docking, loads a TTF font,
// and connects ImGui to the SDL window and SDL_Renderer so it can receive
// input and issue draw calls through the same rendering pipeline the game
// already uses.
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

  // Each of these opens an ImGui window showing a different piece of game
  // state — the player's position and stats, the camera viewport, and a
  // foldable list of every slime currently alive in the scene.
  void drawPlayerInfo(const Player &player);
  void drawCameraInfo(const SDL_FRect &cam);
  void drawSlimesInfo(const std::vector<Slime> &slimes);

  // Called once per frame between SDL_RenderClear and SDL_RenderPresent.
  // newFrame() tells ImGui to start processing input for the new frame,
  // drawFrame() builds all the UI windows and converts them into GPU draw
  // commands, and presentFrame() submits those commands through SDL so the
  // overlay appears on screen.
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
