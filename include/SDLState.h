#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_textengine.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdlib>

struct SDLState {
  SDL_Window *win;
  SDL_Renderer *renderer;
  TTF_Font *font;
  TTF_TextEngine *textEngine;
  const bool *keys;
  int winWidth = 1280, winHeight = 720;
  static constexpr uint32_t logicalWidth = 320, logicalHeight = 180;

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

  // Delete copy constructor and copy assignment operator
  SDLState(const SDLState &) = delete;
  SDLState &operator=(const SDLState &) = delete;

  // Use default move constructor and move assignment operator
  SDLState(SDLState &&) noexcept = default;
  SDLState &operator=(SDLState &&) noexcept = default;

  ~SDLState() {
    TTF_DestroyRendererTextEngine(textEngine);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
  }
};
