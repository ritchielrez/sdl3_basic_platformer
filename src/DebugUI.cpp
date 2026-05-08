#pragma once

#include "DebugUI.h"

#include "Game.h"

void DebugUI::drawPlayerInfo(const Player &player) {
  if (!Game::debug) return;

  ImGui::Begin("Player");
  ImGui::Text("%s", player.inspect().c_str());
  ImGui::End();
}

void DebugUI::drawCameraInfo(const SDL_FRect &cam) {
  if (!Game::debug) return;

  ImGui::Begin("Camera");
  ImGui::Text("%s", fmt::format("Position: ({}, {})\n", cam.x, cam.y).c_str());
  ImGui::End();
}

void DebugUI::newFrame() {
  if (!Game::debug) return;

  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // This ensures the dockspace does not cover the entire screen, so the
  // actual game can seen behind the windows.
  ImGui::DockSpaceOverViewport(0, nullptr,
                               ImGuiDockNodeFlags_PassthruCentralNode);
}

void DebugUI::drawFrame(const Player &player, const SDL_FRect &cam) {
  if (!Game::debug) return;

  drawPlayerInfo(player);
  drawCameraInfo(cam);

  ImGui::Render();
}

void DebugUI::presentFrame() const {
  if (!Game::debug) return;
  SDL_SetRenderLogicalPresentation(sdlState.renderer, 0, 0,
                                   SDL_LOGICAL_PRESENTATION_DISABLED);
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                        sdlState.renderer);
  SDL_SetRenderLogicalPresentation(sdlState.renderer, SDLState::logicalWidth,
                                   SDLState::logicalHeight,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);
}
