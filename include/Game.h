#pragma once

#include "ResourceManager.h"
#include "SDLState.h"
#include "SceneManager.h"

struct Game {
  SDLState sdlState;
  ResourceManager resourceManager;
  SceneManager sceneManager;
  static inline bool debug = false;

  Game(const char *winTitle, SDL_WindowFlags winFlags, const char *rendererName)
      : sdlState(winTitle, winFlags, rendererName),
        resourceManager(sdlState),
        sceneManager(sdlState, resourceManager) {}
};
