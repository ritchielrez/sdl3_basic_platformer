#pragma once

#include <fmt/core.h>

#include <string>

#include "Coin.h"
#include "DynTile.h"
#include "Entity.h"
#include "SDLState.h"
#include "Slime.h"
#include "StaticTile.h"

namespace PlayerAnim {
enum { idle, run, jump, slide, death };
}

struct Player : public Entity {
  glm::vec2 accel, maxSpeed;
  float jumpVel, dashSpeed;
  bool collided, death, grounded, wasGrounded, wasJumpDown, passedCamRuler;
  // Movement mechanic timers
  Timer dashDuration, dashCooldown;
  // Movement polish timers
  Timer coyoteTimer;      // Allow jumping briefly after walking off a ledge
  Timer jumpBufferTimer;  // Buffer a jump pressed just before landing

  Player()
      : accel(glm::vec2(0)),
        maxSpeed(glm::vec2(0)),
        jumpVel(0),
        dashSpeed(0),
        collided(false),
        death(false),
        grounded(false),
        wasGrounded(false),
        wasJumpDown(false),
        passedCamRuler(false),
        dashDuration(0.75f),
        dashCooldown(5.0f),
        coyoteTimer(0.1f),
        jumpBufferTimer(0.12f) {}
  void update(const SDLState& sdlState, SDL_FRect& cam,
              const std::vector<StaticTile>& staticTiles,
              const std::vector<DynTile>& dynTiles, std::vector<Coin>& coins,
              size_t& collectedCoins, std::vector<Slime>& slimes,
              size_t& slainSlimes, float dt);
  void collision(const std::vector<StaticTile>& staticTiles,
                 const std::vector<DynTile>& dynTiles, std::vector<Coin>& coins,
                 size_t& collectedCoins, std::vector<Slime>& slimes,
                 size_t& slainSlimes, float dt);

  [[nodiscard]] std::string inspect() const {
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
      case PlayerAnim::death: {
        playerState = "death";
        break;
      }
    }
    return fmt::format(
        "Position: ({}, {})\nVelocity: ({}, {})\nState: {}\nCollision: "
        "{}\nGrounded: {}\nWas grounded: {}\nPressed jump down (last frame): "
        "{}\nDash duration active: {}\nDash cooldown active: "
        "{}\nCoyote active: {}\nJump buffered: {}\n",
        pos.x, pos.y, vel.x, vel.y, playerState, collided, grounded,
        wasGrounded, wasJumpDown,
        dashDuration.isStarted() && !dashDuration.isTimeOut(),
        dashCooldown.isStarted() && !dashCooldown.isTimeOut(),
        coyoteTimer.isStarted() && !coyoteTimer.isTimeOut(),
        jumpBufferTimer.isStarted() && !jumpBufferTimer.isTimeOut());
  }
};
