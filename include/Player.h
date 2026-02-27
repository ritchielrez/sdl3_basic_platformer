#pragma once

#include <fmt/core.h>

#include <string>

#include "Entity.h"
#include "SDLState.h"

namespace PlayerAnim {
enum { idle, run, jump, slide };
}

struct Player : public Entity {
  glm::vec2 accel;
  float jumpAccel;
  float maxSpeedX;
  bool collided, grounded;

  Player() : accel(glm::vec2(0)), jumpAccel(0), maxSpeedX(0), grounded(false) {}
  void update(const SDLState& sdlState, float dt);
  void collision(float dt);

  void handleKeyInput(const SDLState& sdlState, SDL_Scancode key, bool keyDown);

  std::string inspect() {
    std::string playerState{8, 0};
    switch (currAnim) {
      case PlayerAnim::idle: {
        playerState = "idle";
        break;
      }
      case PlayerAnim::run: {
        playerState = "run";
        break;
      }
      case PlayerAnim::slide: {
        playerState = "slide";
        break;
      }
      case PlayerAnim::jump: {
        playerState = "jump";
        break;
      }
    }
    return fmt::format(
        "Velocity: ({}, {})\nState: {}\nCollision: {}\nGrounded: {}\n", vel.x,
        vel.y, playerState, collided, grounded);
  }
};
