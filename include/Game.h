#pragma once

#include <cstdint>
#include <cstdlib>

#include "ResourceManager.h"
#include "SDL3/SDL_render.h"
#include "SDLState.h"
#include "SceneManager.h"

// Includes `DebugUI` object to show debug information using imgui. Only created
// in debug build. Any methods of `DebugUI` should be only called if the game is
// compiled as a debug build.
#ifdef DEBUG
#include "DebugUI.h"
#endif

struct Game {
  // `SDLState` object which initializes a window and a renderer (allows
  // the user to use GPU to render graphics).
  SDLState sdlState;
  // `ResourceManager` object which initializes textures needed to render
  // game entities.
  ResourceManager resourceManager;
  // `SceneManager` object which manages the current scene (main menu, game,
  // death screen).
  SceneManager sceneManager;
  // `DebugUI` object to show debug information using imgui. Only created in
  // debug build.
#ifdef DEBUG
  DebugUI debugUI{sdlState, "assets/fonts/Roboto-Regular.ttf", 20.0f};
#endif

  // Declare `debug` as static, so any part of the prograqm toggle debug mode.
  static inline bool debug = false;

  Game(const char *winTitle, SDL_WindowFlags winFlags, const char *rendererName)
      : sdlState(winTitle, winFlags, rendererName),
        resourceManager(sdlState),
        sceneManager(sdlState, resourceManager) {
    // Initialize SDL3 for rendering graphics. If initialization fails exit
    // early.
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not initialize SDL3", nullptr);
      exit(1);
    }
  }

  void run() {
    // Keep track of the time when the previous frame started rendering.
    uint64_t prevTime = SDL_GetTicks();
    bool running = true;
    while (running) {
      // Keep track of the time when the current frame starts processing.
      uint64_t nowTime = SDL_GetTicks();
      // Find the delta time, which tells us how much time did it take to
      // process the last frame in seconds.
      float dt = static_cast<float>((nowTime - prevTime)) / 1000.0f;

      SDL_Event event{0};
      // Start looking through all the events that the operating system is
      // trying to send us for processing. These events can be something as
      // basic as mouse clicks, certains keys being pressed, window being
      // resized etc.
      while (SDL_PollEvent(&event)) {
        // Imgui needs to have some control over the event handling if it is
        // being used. This is a requirment of the library.
#ifdef DEBUG
        if (debug) ImGui_ImplSDL3_ProcessEvent(&event);
#endif
        switch (event.type) {
          // If the user clicked on the close button stop running the game.
          case SDL_EVENT_QUIT: {
            running = false;
            break;
          }
          // Keep track of the window width and height internally when it has
          // been resized.
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
            if (event.key.scancode == SDL_SCANCODE_F1) debug = !debug;
#endif
            break;
          }
        }
      }

      // Create a shorthand alias for player.
      Player &player = sceneManager.gameScene.player;

      // Render the debug information on to the screen.
#ifdef DEBUG
      debugUI.newFrame();
      debugUI.drawFrame(player, sceneManager.gameScene.cam);
#endif

      // Clear the screen with black color.
      SDL_SetRenderDrawColor(sdlState.renderer, 0, 0, 0, 255);
      SDL_RenderClear(sdlState.renderer);

      // If the player did not die, keep updating the game state and rendering
      // game entities on to the screen.
      sceneManager.update(dt);
      sceneManager.draw();

      // Show the debug UI on to the screen.
#ifdef DEBUG
      debugUI.presentFrame();
#endif

      // Generate noise and render it on to the screen. This is done after the
      // game entities are rendered so that the noise affects the lighting of
      // the game world. This creates a VHS-style look.
      resourceManager.generateNoise();
      SDL_RenderTexture(sdlState.renderer, resourceManager.getNoiseTex(),
                        nullptr, nullptr);

      // SDL_SetRenderDrawColor(sdlState.renderer, 255, 255, 255, 40);
      // for (uint32_t y = 0; y < SDLState::logicalHeight; y += 4) {
      //   SDL_RenderLine(sdlState.renderer, 0, static_cast<float>(y),
      //                  SDLState::logicalWidth, static_cast<float>(y));
      // }

      // Swap buffers. GPU buffers are general-purpose blocks of memory
      // allocated by the GPU, primarily used to store data for the pixels that
      // are rendered onto the screen.
      SDL_RenderPresent(sdlState.renderer);

      // The current frame has been fully rendered, so this frame's starting
      // time is going to be the previous frame's time relative to the next
      // frame.
      prevTime = nowTime;

      // If the player fall 150 pixels below of the bottom of the screen, they
      // die.
      if (player.pos.y >= SDLState::logicalHeight + 150.0f) {
        player.death = true;
      }
    }
  }

  ~Game() { SDL_Quit(); }
};
