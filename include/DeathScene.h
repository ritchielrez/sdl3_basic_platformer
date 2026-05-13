#pragma once

#include <fmt/format.h>

#include "SDLState.h"
#include "Text.h"

class DeathScene {
  const SDLState &sdlState;
  Text deathText;

 public:
  DeathScene(const SDLState &sdlState)
      : sdlState(sdlState), deathText(sdlState, "You died!!!", glm::vec2(0)) {}

  void update(float dt);
  void draw() { deathText.draw(); };
};
