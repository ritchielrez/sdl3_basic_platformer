#pragma once

#include <cstdint>
#include <cstdlib>

#include "ResourceManager.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDLState.h"
#include "SceneManager.h"

// Includes `DebugUI` object to show debug information using imgui. Only created
// in debug build. Any methods of `DebugUI` should be only called if the game is
// compiled as a debug build.
#ifdef DEBUG
#include "DebugUI.h"
#endif

// Top-level game struct that owns the window, resources, scenes, and the main
// loop. This is the entry point for all game systems — initialization, the
// frame loop, and shutdown.
struct Game {
  // SDL abstraction: window, renderer, font engine, keyboard state.
  SDLState sdlState;
  // All loaded textures (player, tiles, UI) cached in GPU VRAM.
  ResourceManager resourceManager;
  // Scene state machine: start menu → gameplay → death screen.
  SceneManager sceneManager;
#ifdef DEBUG
  // ImGui debug overlay for inspecting entity state, camera, and frame timing
  // at runtime. Only compiled in debug builds to avoid shipping dev tools.
  DebugUI debugUI{sdlState, "assets/fonts/Roboto-Regular.ttf", 20.0f};
#endif

  // Static flag readable from anywhere (including Log::debug). Toggled with F1
  // to show/hide the debug overlay and enable verbose logging.
  static inline bool debug = false;

  // Construct SDL state, load resources, and create the scene state
  // machine. Also initializes SDL video — exits with a message box on
  // failure.
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

  // The main game loop. Each iteration is one frame consisting of:
  //   1. Event pump — process OS input (quit, resize, key events for menus).
  //   2. Debug UI begin — start the ImGui frame.
  //   3. Clear — fill the backbuffer with black.
  //   4. Update — advance game logic by dt seconds.
  //   5. Draw — render the current scene.
  //   6. Debug UI end — render and present the ImGui overlay.
  //   7. Present — swap the front and back buffers (double buffering).
  //
  // Delta time (dt) is computed as the wall-clock time elapsed since the
  // previous frame, clamped to prevent spiral-of-death on lag spikes.
  void run() {
    uint64_t prevTime = SDL_GetTicks();
    bool running = true;
    while (running) {
      uint64_t nowTime = SDL_GetTicks();
      // dt in seconds. If the frame took 16ms, dt ≈ 0.016. Large dt values
      // (e.g. from a debugger breakpoint) are passed through — the physics
      // cap in GameScene::maxPhysicsDt handles excessive time steps.
      float dt = static_cast<float>((nowTime - prevTime)) / 1000.0f;

      // This is a container that SDL fills with info about whatever the
      // player just did — e.g. which key they pressed or where they clicked.
      SDL_Event event{0};
      // Poll all pending OS events. Unlike continuous key state polling
      // (SDL_GetKeyboardState), events are discrete — they fire once per
      // press/release. This makes them ideal for menu navigation and one-shot
      // actions, but not for held-key movement (which uses keyboard state).
      while (SDL_PollEvent(&event)) {
#ifdef DEBUG
        if (debug) ImGui_ImplSDL3_ProcessEvent(&event);
#endif
        // Send the event to the currently active scene (menu, death screen,
        // etc.) so it can react to the key press or mouse click.
        sceneManager.handleEvent(event);

        switch (event.type) {
          // Stop the game when the game window is closed.
          case SDL_EVENT_QUIT: {
            running = false;
            break;
          }
          case SDL_EVENT_WINDOW_RESIZED: {
            // Keep our own copy of the window dimensions so the game can
            // respond to layout changes (e.g. repositioning debug UI).
            sdlState.winWidth = event.window.data1;
            sdlState.winHeight = event.window.data2;
            break;
          }
          case SDL_EVENT_KEY_DOWN: {
#ifdef DEBUG
            // Toggle debug overlay when F1 pressed.
            if (event.key.scancode == SDL_SCANCODE_F1) debug = !debug;
#endif
            break;
          }
        }
      }

      // If the user clicks on quit button on the start scene, close the game.
      if (sceneManager.startScene.shouldQuit) {
        running = false;
      }

      // Draw debug overlay on debug builds.
#ifdef DEBUG
      debugUI.newFrame();
      debugUI.drawFrame(sceneManager.gameScene.player,
                        sceneManager.gameScene.slimes,
                        sceneManager.gameScene.cam);
#endif

      // Clear to black before drawing the new frame. Without this, the
      // previous frame's pixels would remain on screen, causing ghosting.
      SDL_SetRenderDrawColor(sdlState.renderer, 0, 0, 0, 255);
      SDL_RenderClear(sdlState.renderer);

      //
      sceneManager.update(dt);
      sceneManager.draw();

#ifdef DEBUG
      debugUI.presentFrame();
#endif

      // Double buffering: the renderer draws to a hidden backbuffer while the
      // frontbuffer is displayed. SDL_RenderPresent swaps them atomically on
      // the vertical blank (when VSync is enabled), preventing visible tearing.
      SDL_RenderPresent(sdlState.renderer);

      prevTime = nowTime;
    }
  }

  // Cleanup SDL related data at the end of the game.
  ~Game() { SDL_Quit(); }
};
