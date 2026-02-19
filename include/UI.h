#pragma once

#include "SDLState.h"

struct UI {
  ImGuiIO io;
  ImGuiStyle style;

  UI(const SDLState &sdlState) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui_ImplSDL3_InitForSDLRenderer(sdlState.win, sdlState.renderer);
    ImGui_ImplSDLRenderer3_Init(sdlState.renderer);

    io.Fonts->AddFontFromFileTTF("assets/fonts/PixelOperator8.ttf");
  }
  ~UI() {
    ImGui_ImplSDLRenderer3_Shutdown(); 
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext(););
  }
};
