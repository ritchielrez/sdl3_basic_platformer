#pragma once

#include <fmt/core.h>

#include <string>

#include "Coin.h"
#include "DynTile.h"
#include "Enemy.h"
#include "Entity.h"
#include "SDLState.h"
#include "StaticTile.h"

namespace PlayerAnim {
enum { idle, run, jump, slide };
}

struct Player : public Entity {
  glm::vec2 accel;
  float jumpVel, gravVel;
  float maxSpeedX;
  bool collided, death, grounded, passedCamRuler;

  Player()
      : accel(glm::vec2(0)),
        jumpVel(0),
        gravVel(0),
        maxSpeedX(0),
        collided(false),
        death(false),
        grounded(false),
        passedCamRuler(false) {}
  void update(const SDLState& sdlState, SDL_FRect& cam,
              const std::vector<StaticTile>& staticTiles,
              const std::vector<DynTile>& dynTiles, std::vector<Coin>& coins,
              size_t& collectedCoins, const std::vector<Enemy>& enemies,
              float dt);
  void collision(const std::vector<StaticTile>& staticTiles,
                 const std::vector<DynTile>& dynTiles, std::vector<Coin>& coins,
                 size_t& collectedCoins, const std::vector<Enemy>& enemies);

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
    }
    return fmt::format(
        "Position: ({}, {})\nVelocity: ({}, {})\nState: {}\nCollision: "
        "{}\nGrounded: {}\n",
        pos.x, pos.y, vel.x, vel.y, playerState, collided, grounded);
  }
};
