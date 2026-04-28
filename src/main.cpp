#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <fmt/base.h>

// Only include imgui for debug builds because imgui is only used to show
// debugging information. Dear imgui is a bloat-free graphical user interface
// library for C++. It is fast, portable, renderer agnostic, and self-contained
// (no external dependencies). It has been used by various AAA companies to
// build various in game proprietary tools.
#ifdef DEBUG
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

#include "DebugUI.h"
#endif

#include <cassert>
// glm is a C++ math library offering important math objects such as vectors,
// matrices etc.
#include <glm/glm.hpp>

#include "Game.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "defer.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  // Initialize SDL3 for rendering graphics. If initialization fails exit early.
  // SDL3 is a library that can handle audio, input and graphics accross various
  // platforms. One of the biggest users of SDL is Valve, as many of their games
  // such as Team Fortress 2 use SDL for some parts of the games.
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Could not initialize SDL3", nullptr);
    return 1;
  }
  // Deinitialize SDL3 at the very end of the program. Important as SDL
  // allocates bunch of stuff on the memory and they need to be freed in the end
  // to ensure there are no memory leaks.
  defer(SDL_Quit());

  // Create `SDLState` object which initializes a window and a renderer (allows
  // the user to use GPU to render graphics).
  SDLState sdlState{"sdl3_basic_platformer", SDL_WINDOW_RESIZABLE, nullptr};

  // Create `ResourceManager` object which initializes textures needed to render
  // game entities.
  ResourceManager resourceManager{sdlState};

  // Create `Game` object which initializes game entities.
  Game game;
  game.init(sdlState, resourceManager);

  // Create `DebugUI` object to show debug information using imgui.
#ifdef DEBUG
  DebugUI debugUI{sdlState, "assets/fonts/Roboto-Regular.ttf", 20.0f};
#endif

  // Keep track of the time when the previous frame started rendering.
  uint64_t prevTime = SDL_GetTicks();
  bool running = true;
  while (running) {
    // Keep track of the time when the current frame starts processing.
    uint64_t nowTime = SDL_GetTicks();
    // Find the delta time, which tells us how much time did it take to process
    // the last frame in seconds.
    float dt = (nowTime - prevTime) / 1000.0f;

    SDL_Event event{0};
    // Start looking through all the events that the operating system is trying
    // to send us for processing. These events can be something as basic as
    // mouse clicks, certains keys being pressed, window being resized etc.
    while (SDL_PollEvent(&event)) {
      // Imgui needs to have some control over the event handling if it is being
      // used. This is a requirment of the library.
#ifdef DEBUG
      if (game.debug) ImGui_ImplSDL3_ProcessEvent(&event);
#endif
      switch (event.type) {
        // If the user clicked on the close button stop running the game.
        case SDL_EVENT_QUIT: {
          running = false;
          break;
        }
        // Keep track of the window width and height internally when it has been
        // resized.
        case SDL_EVENT_WINDOW_RESIZED: {
          sdlState.winWidth = event.window.data1;
          sdlState.winHeight = event.window.data2;
          break;
        }
        // Handle when keys are being pressed down by the user.
        // NOTE: Keyboard events are better if you do not want multiple key
        // down events in a single frame. With events, there is a delay
        // between consecutive key down events.
        case SDL_EVENT_KEY_DOWN: {
          // Toggle showing debug information when the
          // user presses F1, only if a debug build is being run.
#ifdef DEBUG
          if (event.key.scancode == SDL_SCANCODE_F1) game.debug = !game.debug;
#endif
          break;
        }
      }
    }

    // Render the debug information on to the screen.
#ifdef DEBUG
    debugUI.newFrame();
    debugUI.drawFrame();
#endif

    // Clear the screen with black color.
    SDL_SetRenderDrawColor(sdlState.renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlState.renderer);

    // If the player dies, restart the game and print "You died." (for now).
    // TODO: Implement game over screen.
    if (game.player.death) {
      game.reset(sdlState, resourceManager);
      fmt::println("You died.");
    } else {
      // If the player did not die, keep updating the game state and rendering
      // game entities on to the screen.
      game.update(sdlState, dt);
      game.draw(sdlState);
    }

#ifdef DEBUG
    debugUI.presentFrame(sdlState);
#endif

    // Swap buffers. GPU buffers are general-purpose blocks of memory allocated
    // by the GPU, primarily used to store data for the pixels that are rendered
    // onto the screen.
    SDL_RenderPresent(sdlState.renderer);

    // The current frame has been fully rendered, so this frame's starting time
    // is going to be the previous frame's time relative to the next frame.
    prevTime = nowTime;

    // If the player fall 150 pixels below of the bottom of the screen, they
    // die.
    if (Game::player.pos.y >= sdlState.logicalHeight + 150.0f) {
      game.player.death = true;
    }
  }

  return 0;
}
