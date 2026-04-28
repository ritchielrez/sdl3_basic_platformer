#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_textengine.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdlib>

// `SDLState` is a class holding internal data that are needed for SDL to
// function.
struct SDLState {
  SDL_Window *win;
  // `SDL_Renderer` is a simple 2D rendering API. This handles cross platform
  // rendering of graphics.
  SDL_Renderer *renderer;
  // `TTF_Font` is a class that allows a certain font to be used for text
  // rendering.
  TTF_Font *font;
  // `TTF_TextEngine` is a class that allows text to be rendered with dynamic
  // layouts.
  TTF_TextEngine *textEngine;
  // This holds an array of the state of each key of the keyboard.
  const bool *keys;
  // Set the default window size to be 1280x720.
  int winWidth = 1280, winHeight = 720;
  // However, internally the game is going to be rendered at 320x180.
  // Because this is a pixel art game though the upscaling is not going to cause
  // any blurriness, so many pixel art games actually render at low resolutions
  // like 180p.
  static constexpr uint32_t logicalWidth = 320, logicalHeight = 180;

  // No argument constructor setting everything to the default value of
  // `nullptr`. This does not construct any SDL related objects.
  SDLState()
      : win(nullptr),
        renderer(nullptr),
        font(nullptr),
        textEngine(nullptr),
        keys(nullptr) {}
  // Parameterized constructor to initialize everything properly. This does
  // construct the necessary SDL related objects.
  SDLState(const char *winTitle, SDL_WindowFlags winFlags,
           const char *rendererName) {
    // Create the game window, if it fails then exit early.
    win = SDL_CreateWindow(winTitle, winWidth, winHeight, winFlags);
    if (!win) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not create window", nullptr);
      exit(1);
    }

    // Construct a `SDL_Renderer` object, if it fails then exit early.
    renderer = SDL_CreateRenderer(win, rendererName);
    if (!renderer) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not create renderer", nullptr);
      exit(1);
    }

    // Set the logical resolution of the window. Ensure that the game only
    // upscaled by integer multiples.
    SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    // Tell SDL how to use the alpha values, so transparency is enabled.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Ask SDL about the keyboard state, so basically which keys are being
    // pressed and which are not.
    keys = SDL_GetKeyboardState(nullptr);

    // Initialize a `TTF_Font` object, if it fails then exit early.
    if (!TTF_Init()) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not initialize SDL_ttf", nullptr);
      exit(1);
    }

    // Open a font file so it can be used for text rendering. Set the font size
    // to be 8pt.
    font = TTF_OpenFont("assets/fonts/PixelOperator8.ttf", 8.0f);
    if (!font) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not open font", nullptr);
      exit(1);
    }

    // Initialize a `TTF_TextEngine` object, if it fails then exit early.
    textEngine = TTF_CreateRendererTextEngine(renderer);
    if (!textEngine) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not create text engine", nullptr);
      exit(1);
    }
  }

  // Delete copy constructor and copy assignment operator. This way, a
  // `SDLState` object cannot be copied.
  SDLState(const SDLState &) = delete;
  SDLState &operator=(const SDLState &) = delete;

  // Use default move constructor and move assignment operator. This way,
  // `SDLState` object can only be moved, because I do not want to have any
  // copies of internal SDL data. Having copies may lead to confusion.
  SDLState(SDLState &&) noexcept = default;
  SDLState &operator=(SDLState &&) noexcept = default;

  // Destructor to deinitialize any SDL related data. This prevents any memory
  // leaks.
  ~SDLState() {
    TTF_DestroyRendererTextEngine(textEngine);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
  }
};
