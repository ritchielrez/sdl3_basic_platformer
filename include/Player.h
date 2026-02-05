#pragma once

#include "Entity.h"
#include "SDLState.h"

namespace PlayerAnim {
enum Anims { idle, run };
}

struct Player : public Entity {
  glm::vec2 accel;
  float maxSpeedX;

  Player() : accel(glm::vec2(0)), maxSpeedX(0) {}
  void update(const SDLState& sdlState, float dt);
  void collision(float dt);
};
