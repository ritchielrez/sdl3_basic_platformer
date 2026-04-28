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
  TTF_Font *font;
  TTF_TextEngine *textEngine;
  const bool *keys;
  // The difference between normal window width and height and the logical width
  // and height is
  int winWidth = 1280, winHeight = 720;
  static constexpr uint32_t logicalWidth = 320, logicalHeight = 180;

  // No argument constructor setting everything to the default value of
  // `nullptr`.
  SDLState()
      : win(nullptr),
        renderer(nullptr),
        font(nullptr),
        textEngine(nullptr),
        keys(nullptr) {}
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

    SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    // Tell SDL how to use the alpha values.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    keys = SDL_GetKeyboardState(nullptr);

    if (!TTF_Init()) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not initialize SDL_ttf", nullptr);
      exit(1);
    }

    font = TTF_OpenFont("assets/fonts/PixelOperator8.ttf", 8.0f);
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

  // Delete copy constructor and copy assignment operator. This way, a
  // `SDLState` object cannot be copied.
  SDLState(const SDLState &) = delete;
  SDLState &operator=(const SDLState &) = delete;

  // Use default move constructor and move assignment operator. This way,
  // `SDLState` object can only be moved, because I do not want to have any
  // copies of internal SDL data. Having copies may lead to confusion.
  SDLState(SDLState &&) noexcept = default;
  SDLState &operator=(SDLState &&) noexcept = default;

  // Destructor to deinitialize any SDL related data.
  ~SDLState() {
    TTF_DestroyRendererTextEngine(textEngine);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
  }
};
