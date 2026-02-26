#pragma once

#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

#include <functional>
#include <string>

#include "Game.h"
#include "SDLState.h"

struct UI {
  ImGuiIO io;
  ImGuiStyle style;

  float fontHeight;

  UI() = delete;

  UI(const SDLState &sdlState, const std::string_view fontPath) {
    fontHeight = 18.0f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui_ImplSDL3_InitForSDLRenderer(sdlState.win, sdlState.renderer);
    ImGui_ImplSDLRenderer3_Init(sdlState.renderer);

    io.Fonts->AddFontFromFileTTF(fontPath.data(), fontHeight);
  }
  UI(const SDLState &sdlState, const std::string_view fontPath,
     const float fontHeight) {
    this->fontHeight = fontHeight;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui_ImplSDL3_InitForSDLRenderer(sdlState.win, sdlState.renderer);
    ImGui_ImplSDLRenderer3_Init(sdlState.renderer);

    io.Fonts->AddFontFromFileTTF(fontPath.data(), fontHeight);
  }

  void newFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // This ensures the dockspace does not cover the entire screen, so the
    // actual game can seen behind the windows.
    ImGui::DockSpaceOverViewport(0, nullptr,
                                 ImGuiDockNodeFlags_PassthruCentralNode);
  }

  void drawPlayerInfo() {
    ImGui::Begin("Player");
    {
      std::string playerState{8, 0};
      switch (Game::player.currAnim) {
        case PlayerAnim::idle: {
          playerState = "idle";
          break;
        }
        case PlayerAnim::run: {
          playerState = "run";
          break;
        }
        case PlayerAnim::jump: {
          playerState = "jump";
          break;
        }
      }
      ImGui::Text("Velocity: (%f, %f)", Game::player.vel.x, Game::player.vel.y);
      ImGui::Text("State: %s", playerState.c_str());
    }
    ImGui::End();
  }

  void drawPlayerInfoCustom(std::function<void()> drawFunc) {
    newFrame();
    ImGui::Begin("Player");
    {
      drawFunc();
    }
    ImGui::End();
  }

  void drawFrame() {
    drawPlayerInfo();

    ImGui::Render();
  }

  void presentFrame(const SDLState &sdlState) {
    SDL_SetRenderLogicalPresentation(sdlState.renderer, 0, 0,
                                     SDL_LOGICAL_PRESENTATION_DISABLED);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                          sdlState.renderer);
    SDL_SetRenderLogicalPresentation(sdlState.renderer, sdlState.logWidth,
                                     sdlState.logHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
  }

  ~UI() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }
};
