#pragma once

#include "Entity.h"
#include "SDLState.h"

namespace PlayerAnim {
enum Anims { idle, run, jump };
}

struct Player : public Entity {
  glm::vec2 accel;
  float jumpAccel;
  float maxSpeedX;
  bool grounded;

  Player() : accel(glm::vec2(0)), jumpAccel(0), maxSpeedX(0), grounded(false) {}
  void update(const SDLState& sdlState, float dt);
  void collision(float dt);

  void handleKeyInput(const SDLState& sdlState, SDL_Scancode key, bool keyDown);
};
