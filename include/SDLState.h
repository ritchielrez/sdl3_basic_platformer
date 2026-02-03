#pragma once

#include <SDL3/SDL.h>

struct SDLState {
  SDL_Window *win;
  SDL_Renderer *renderer;
  const bool *keys;
  uint32_t winWidth = 1280, winHeight = 720;
  static constexpr uint32_t logWidth = 640, logHeight = 360;

  SDLState() : win(nullptr), renderer(nullptr), keys(nullptr) {}
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

    SDL_SetRenderLogicalPresentation(renderer, logWidth, logHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    // Tell SDL how to use the alpha values.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    keys = SDL_GetKeyboardState(nullptr);
  }
  ~SDLState() {
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
  }
};
