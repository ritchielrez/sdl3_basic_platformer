#pragma once

#include "ResourceManager.h"
#include "SDLState.h"
#include "SceneManager.h"

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
  static inline bool debug = false;

  Game(const char *winTitle, SDL_WindowFlags winFlags, const char *rendererName)
      : sdlState(winTitle, winFlags, rendererName),
        resourceManager(sdlState),
        sceneManager(sdlState, resourceManager) {}
};
