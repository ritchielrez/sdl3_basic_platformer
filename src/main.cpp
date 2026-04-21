#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <fmt/base.h>

#ifdef DEBUG
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

#include "DebugUI.h"
#endif

#include <cassert>
#include <glm/glm.hpp>
#include <string_view>

#include "Game.h"
#include "Log.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "defer.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Could not initialize SDL3", nullptr);
    return 1;
  }
  defer(SDL_Quit());

  SDLState sdlState{"sdl3_basic_platformer", SDL_WINDOW_RESIZABLE, nullptr};
  ResourceManager resourceManager{sdlState};
  Game game;
  game.init(sdlState, resourceManager);

#ifdef DEBUG
  DebugUI debugUI{sdlState, "assets/fonts/Roboto-Regular.ttf", 20.0f};
#endif

  uint64_t prevTime = SDL_GetTicks();
  bool running = true;
  while (running) {
    uint64_t nowTime = SDL_GetTicks();
    float dt = (nowTime - prevTime) / 1000.0f;

    SDL_Event event{0};
    while (SDL_PollEvent(&event)) {
#ifdef DEBUG
      if (game.debug) ImGui_ImplSDL3_ProcessEvent(&event);
#endif
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
#ifdef DEBUG
          if (event.key.scancode == SDL_SCANCODE_F1) game.debug = !game.debug;
#endif
          break;
        }
      }
    }

#ifdef DEBUG
    debugUI.newFrame();
    debugUI.drawFrame();
#endif

    // Clear screen
    SDL_SetRenderDrawColor(sdlState.renderer, 20, 152, 220, 255);
    SDL_RenderClear(sdlState.renderer);

    if (game.player.death) {
      game.reset(sdlState, resourceManager);
      fmt::println("You died.");
    } else {
      game.update(sdlState, dt);
      game.draw(sdlState);
    }

#ifdef DEBUG
    debugUI.presentFrame(sdlState);
#endif

    // Swap buffers
    SDL_RenderPresent(sdlState.renderer);

    prevTime = nowTime;

    if (Game::player.pos.y >= sdlState.logicalHeight + 150.0f) {
      game.player.death = true;
    }
  }

  return 0;
}
