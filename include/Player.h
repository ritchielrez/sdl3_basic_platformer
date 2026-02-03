#pragma once

#include "Entity.h"
#include "SDLState.h"

namespace PlayerAnim {
enum Anims { idle, run };
}

struct Player : public Entity {
  glm::vec2 accel;

  Player() : accel(glm::vec2(0)) {}
  void update(const SDLState& sdlState, float dt);
  void collision(float dt);
};
