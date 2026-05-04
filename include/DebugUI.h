#pragma once
#include "Player.h"
#ifdef DEBUG
#include <backends/imgui_impl_sdl3.h>
#include <fmt/core.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

#include <string>

#include "Game.h"
#include "SDLState.h"

struct DebugUI {
  float fontHeight;

  DebugUI() = delete;

  DebugUI(const SDLState &sdlState, const std::string_view &fontPath) {
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
          const float fontHeight) {
    this->fontHeight = fontHeight;

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

  void newFrame() {
    if (!Game::debug) return;

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // This ensures the dockspace does not cover the entire screen, so the
    // actual game can seen behind the windows.
    ImGui::DockSpaceOverViewport(0, nullptr,
                                 ImGuiDockNodeFlags_PassthruCentralNode);
  }

  void drawPlayerInfo(const Player &player) {
    if (!Game::debug) return;

    ImGui::Begin("Player");
    ImGui::Text("%s", player.inspect().c_str());
    ImGui::End();
  }

  void drawCameraInfo() {
    if (!Game::debug) return;

    ImGui::Begin("Camera");
    ImGui::Text(
        "%s",
        fmt::format("Position: ({}, {})\n", Game::cam.x, Game::cam.y).c_str());
    ImGui::End();
  }

  void drawFrame() {
    if (!Game::debug) return;

    drawPlayerInfo();
    drawCameraInfo();

    ImGui::Render();
  }

  void presentFrame(const SDLState &sdlState) {
    if (!Game::debug) return;
    SDL_SetRenderLogicalPresentation(sdlState.renderer, 0, 0,
                                     SDL_LOGICAL_PRESENTATION_DISABLED);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                          sdlState.renderer);
    SDL_SetRenderLogicalPresentation(sdlState.renderer, sdlState.logicalWidth,
                                     sdlState.logicalHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
  }

  ~DebugUI() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }
};
#endif
