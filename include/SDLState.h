#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_textengine.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdlib>

#include "SDL3/SDL_render.h"

// Holds all SDL3 subsystems needed to run the game: window, renderer, font
// engine, and keyboard state. Initialization order matters — some subsystems
// depend on others (e.g. the text engine needs a renderer). Acts as the single
// point of contact with the underlying graphics API.
struct SDLState {
  SDL_Window *win;
  // SDL_Renderer is the cross-platform 2D rendering API. It abstracts over
  // Direct3D, OpenGL, Metal, and Vulkan so the same draw calls work everywhere.
  SDL_Renderer *renderer;
  // Loaded TrueType font, used by the text engine for all UI text.
  TTF_Font *font;
  // Font size in points. 8pt works well for pixel-art text at 320×180.
  static constexpr float fontSize = 8.0f;
  // TTF_TextEngine handles text layout (kerning, wrapping) and renders glyphs
  // into textures via the renderer.
  TTF_TextEngine *textEngine;
  // Array of keyboard scancode states obtained from SDL. Each element is true
  // while the corresponding key is held down. Indexed by SDL_SCANCODE_* values.
  const bool *keys;
  // Window dimensions in physical (monitor) pixels. The window starts at
  // 1280×720 but can be resized at runtime.
  int winWidth = 1280, winHeight = 720;
  // Internal logical resolution: 320×180. The game world is rendered at this
  // low resolution, then upscaled to the window via SDL_LOGICAL_PRESENTATION_INTEGER_SCALE.
  // Pixel art looks crisp with nearest-neighbor upscaling because there is no
  // sub-pixel blending. The 16:9 aspect ratio (320/180 = 16/9) matches common
  // display ratios.
  static constexpr uint16_t logicalWidth = 320, logicalHeight = 180;

  // Default constructor — all pointers null. Exists so SDLState can be
  // declared before the real constructor runs.
  SDLState()
      : win(nullptr),
        renderer(nullptr),
        font(nullptr),
        textEngine(nullptr),
        keys(nullptr) {}
  // Full initializer: creates window, renderer, loads font, sets up text
  // engine, and queries keyboard state. Exits with an error dialog if any
  // critical step fails — there is no point running without a window or
  // renderer.
  SDLState(const char *winTitle, SDL_WindowFlags winFlags,
           const char *rendererName) {
    win = SDL_CreateWindow(winTitle, winWidth, winHeight, winFlags);
    if (!win) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not create window", nullptr);
      exit(1);
    }

    renderer = SDL_CreateRenderer(win, rendererName);
    if (!renderer) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not create renderer", nullptr);
      exit(1);
    }

    // Adaptive VSync tells the renderer to wait for the monitor's vertical
    // blank before swapping buffers, eliminating screen tearing. "Adaptive"
    // means it also tries to match the monitor's refresh rate dynamically.
    SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

    // Set logical resolution to 320×180 with integer scaling. This means:
    // 1. All draw calls use the small coordinate space (320×180).
    // 2. SDL automatically upscales the final buffer to the window size.
    // 3. Integer scaling prevents pixel distortion (each game pixel maps to
    //    an integer number of screen pixels, e.g. 4×4 at 1280×720).
    SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    // Enable per-pixel alpha blending so translucent elements (fading UI,
    // particle effects) composite correctly over the scene.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Retrieve a snapshot of the keyboard state. SDL_GetKeyboardState returns
    // a pointer to an internal array that is live-updated as keys are pressed
    // and released. This is more responsive than polling per-event for
    // continuous input like running.
    keys = SDL_GetKeyboardState(nullptr);

    if (!TTF_Init()) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not initialize SDL_ttf", nullptr);
      exit(1);
    }

    // PixelOperator8-Bold is a pixel-art monospace font that matches the
    // 320×180 aesthetic. Size 8pt keeps text readable without consuming
    // precious screen real estate.
    font = TTF_OpenFont("assets/fonts/PixelOperator8-Bold.ttf", fontSize);
    if (!font) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not open font", nullptr);
      exit(1);
    }

    textEngine = TTF_CreateRendererTextEngine(renderer);
    if (!textEngine) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not create text engine", nullptr);
      exit(1);
    }
  }

  // Non-copyable: there must only be one owner of SDL resources (window,
  // renderer). Copying would lead to double-free on destruction.
  SDLState(const SDLState &) = delete;
  SDLState &operator=(const SDLState &) = delete;

  // Movable: ownership of SDL handles can be transferred (e.g. when storing
  // SDLState in a struct). The moved-from object becomes null.
  SDLState(SDLState &&) noexcept = default;
  SDLState &operator=(SDLState &&) noexcept = default;

  // Tear down SDL subsystems in reverse order of creation. Each Destroy/Close
  // call releases internal resources held by the corresponding subsystem.
  ~SDLState() {
    TTF_DestroyRendererTextEngine(textEngine);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
  }
};
