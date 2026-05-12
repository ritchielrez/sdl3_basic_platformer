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

  void update([[maybe_unused]] float dt) { deathText.draw(); }
  void draw();
};
