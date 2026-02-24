#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

#include <array>
#include <cassert>
#include <glm/glm.hpp>
#include <string_view>
#include <vector>

#include "DynTile.h"
#include "Game.h"
#include "Log.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StaticTile.h"
#include "UI.h"
#include "defer.h"

int main(int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Could not initialize SDL3", nullptr);
    return 1;
  }
  defer(SDL_Quit());

  SDLState sdlState{"learn_sdl3", SDL_WINDOW_RESIZABLE, nullptr};
  ResourceManager resourceManager{sdlState};

  Game::createPlayer(sdlState, resourceManager);
  Game::loadTileMap(sdlState, resourceManager);

  UI ui{sdlState, 14.0f};

  uint64_t prevTime = SDL_GetTicks();
  bool running = true;
  while (running) {
    uint64_t nowTime = SDL_GetTicks();
    float dt = (nowTime - prevTime) / 1000.0f;

    SDL_Event event{0};
    while (SDL_PollEvent(&event)) {
      if (Game::debug) ImGui_ImplSDL3_ProcessEvent(&event);
      switch (event.type) {
        case SDL_EVENT_QUIT: {
          running = false;
          break;
        }
        case SDL_EVENT_WINDOW_RESIZED: {
          sdlState.winWidth = event.window.data1;
          sdlState.winHeight = event.window.data2;
          break;
        }
        // NOTE: Keyboard events are better if you do not want multiple key
        // down events in a single frame. With events, there is a delay
        // between consecutive key down events.
        case SDL_EVENT_KEY_DOWN: {
          if (event.key.scancode == SDL_SCANCODE_F1) Game::debug = !Game::debug;
          Game::player.handleKeyInput(sdlState, event.key.scancode, true);
          break;
        }
        case SDL_EVENT_KEY_UP: {
          Game::player.handleKeyInput(sdlState, event.key.scancode, false);
          break;
        }
      }
    }

    if (Game::debug) {
      ui.newFrame();
      ui.drawFrame();
    }

    // Clear screen
    SDL_SetRenderDrawColor(sdlState.renderer, 20, 152, 220, 255);
    SDL_RenderClear(sdlState.renderer);

    // Update

    // Only animate the player if the current animation has multiple frames.
    // If it has one frame, the timer length/duration is set to 0.
    if (Game::player.anims[Game::player.currAnim].getLen() != 0) {
      Game::player.anims[Game::player.currAnim].step(dt);
    }
    Game::player.update(sdlState, dt);

    // Render
    Game::player.draw(sdlState);
    for (auto &staticTile : Game::staticTiles) {
      staticTile.draw(sdlState);
    }
    for (auto &dynTile : Game::dynTiles) {
      dynTile.draw(sdlState);
    }

    if (Game::debug) {
      ui.presentFrame(sdlState);
    }

    // Swap buffers
    SDL_RenderPresent(sdlState.renderer);

    prevTime = nowTime;
  }

  return 0;
}
