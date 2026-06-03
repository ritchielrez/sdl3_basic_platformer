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
// top of the game. fontHeight controls the size of the UI text.
struct DebugUI {
  const SDLState &sdlState;
  float fontHeight;

  // No default no argument constructor — an SDLState and font path are always required.
  DebugUI() = delete;

  // Constructor using a default 18px font size. Creates an ImGui context,
  // enables keyboard navigation and window docking, applies the dark colour
  // scheme, hooks ImGui into the existing SDL window and SDL_Renderer so it
  // can receive input and issue draw calls, then loads a TTF font from disk.
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
  // Constructor that accepts an explicit font size.  Otherwise identical to
  // the one above — stores the given fontHeight instead of using 18px.
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

  // Opens an ImGui window titled "Player" showing the player's position,
  // velocity, health, and other internal state from Player::inspect().
  void drawPlayerInfo(const Player &player);

  // Opens an ImGui window titled "Camera" showing the viewport's current
  // (x, y) position in the world.
  void drawCameraInfo(const SDL_FRect &cam);

  // Opens an ImGui window titled "Slimes" listing every slime enemy alive in
  // the scene. Each slime is shown in a foldable section (CollapsingHeader)
  // to keep the list compact when there are many enemies.
  void drawSlimesInfo(const std::vector<Slime> &slimes);

  // Starts a fresh ImGui frame. Must be called after SDL_RenderClear and
  // before any ImGui windows are created. Resets the renderer backend, input
  // backend, and core ImGui state, then creates a transparent full-screen
  // docking area so debug windows can be arranged freely without blocking
  // clicks on the game beneath.
  void newFrame();

  // Builds every debug window by calling the three draw*Info methods, then
  // calls ImGui::Render() to bake the UI into GPU draw commands.
  void drawFrame(const Player &player, const std::vector<Slime> &slimes,
                 const SDL_FRect &cam);

  // Submits the buffered ImGui draw commands through SDL so the overlay
  // appears on screen. Temporarily disables logical presentation first
  // because the game uses a fixed internal resolution — ImGui expects raw
  // window pixel coordinates, not the scaled game resolution.
  void presentFrame() const;

  // Shuts down the ImGui-SDL backends and destroys the ImGui context,
  // releasing all GPU resources and UI state.
  ~DebugUI() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }
};
#endif
