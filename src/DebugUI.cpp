#include "DebugUI.h"

#include "Game.h"

// Opens an ImGui window titled "Player" and dumps the player's internal state
// (position, velocity, health, etc.) as formatted text from Player::inspect().
// ImGui::Begin / End create a draggable, resizable window; every widget call
// between them is placed inside that window.
void DebugUI::drawPlayerInfo(const Player &player) {
  if (!Game::debug) return;

  ImGui::Begin("Player");
  ImGui::Text("%s", player.inspect().c_str());
  ImGui::End();
}

// Shows the camera's current (x, y) position.  The camera is an SDL_FRect
// defining which portion of the world is visible — it acts like a viewport
// that follows the player around the level.

void DebugUI::drawCameraInfo(const SDL_FRect &cam) {
  if (!Game::debug) return;

  ImGui::Begin("Camera");
  ImGui::Text("Position: (%f, %f)", cam.x, cam.y);
  ImGui::End();
}

// Lists every slime enemy currently alive.  Shows the total count first, then
// each slime in a foldable section (ImGui::CollapsingHeader) so the list
// stays compact when there are many enemies on screen.
void DebugUI::drawSlimesInfo(const std::vector<Slime> &slimes) {
  if (!Game::debug) return;

  ImGui::Begin("Slimes");
  ImGui::Text("Total slimes: %zu\n", slimes.size());
  for (size_t i = 0; i < slimes.size(); ++i) {
    std::string label = fmt::format("Slime {}", i);
    if (ImGui::CollapsingHeader(label.c_str())) {
      ImGui::Text("%s", slimes[i].inspect().c_str());
    }
  }
  ImGui::End();
}

// Prepares ImGui to receive input and build UI for the current frame.  Must
// be called after SDL_RenderClear and before any ImGui windows are created.
// The three NewFrame calls reset each layer of ImGui's internal state (the
// SDL renderer backend, the SDL input backend, and core ImGui).  Afterwards
// DockSpaceOverViewport creates a full-screen docking area; the
// PassthruCentralNode flag keeps the centre of the screen transparent so
// mouse clicks and keyboard input pass through to the game underneath.
void DebugUI::newFrame() {
  if (!Game::debug) return;

  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(0, nullptr,
                               ImGuiDockNodeFlags_PassthruCentralNode);
}

// Builds every debug window by calling the sub-panel functions, then calls
// ImGui::Render() to bake the window descriptions into GPU draw commands
// (vertex buffers, textures, etc.).  Those commands are stored internally and
// submitted to the GPU during presentFrame().
void DebugUI::drawFrame(const Player &player, const std::vector<Slime> &slimes,
                        const SDL_FRect &cam) {
  if (!Game::debug) return;

  drawPlayerInfo(player);
  drawSlimesInfo(slimes);
  drawCameraInfo(cam);

  ImGui::Render();
}

// Submits the ImGui draw data to SDL so the overlay appears on screen.
// Logical presentation is temporarily disabled beforehand because the game
// uses a fixed internal resolution (e.g. 640×360) that SDL scales and
// letterboxes to fit the window — ImGui expects raw pixel coordinates matching
// the real window size, not the scaled game resolution.
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
