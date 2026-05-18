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
  ImGui::Text("Position: (%f, %f)", cam.x, cam.y);
  ImGui::End();
}

void DebugUI::drawEnemiesInfo(const std::vector<Enemy> &enemies) {
  if (!Game::debug) return;

  ImGui::Begin("Enemies");
  ImGui::Text("Total enemies: %zu\n", enemies.size());
  for (size_t i = 0; i < enemies.size(); ++i) {
    std::string label = fmt::format("Enemy {}", i);
    if (ImGui::CollapsingHeader(label.c_str())) {
      ImGui::Text("%s", enemies[i].inspect().c_str());
    }
  }
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

void DebugUI::drawFrame(const Player &player, const std::vector<Enemy> &enemies,
                        const SDL_FRect &cam) {
  if (!Game::debug) return;

  drawPlayerInfo(player);
  drawEnemiesInfo(enemies);
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
