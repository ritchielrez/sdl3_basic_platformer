#include "Player.h"

#include <SDL3/SDL.h>

#include "Coin.h"
#include "DynTile.h"
#include "Enemy.h"
#include "StaticTile.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include "SDLState.h"

void Player::update(const SDLState& sdlState, SDL_FRect& cam,
                    const std::vector<StaticTile>& staticTiles,
                    const std::vector<DynTile>& dynTiles,
                    std::vector<Coin>& coins, size_t& collectedCoins,
                    const std::vector<Enemy>& enemies, float dt) {
  float currDir = 0;
  if (sdlState.keys[SDL_SCANCODE_A]) {
    currDir -= 1;
  }
  if (sdlState.keys[SDL_SCANCODE_D]) {
    currDir += 1;
  }
  if (currDir != 0) {
    dir = currDir;
  }

  if (grounded && sdlState.keys[SDL_SCANCODE_SPACE]) {
    vel.y = jumpVel;
    currAnim = PlayerAnim::jump;
  }

  switch (currAnim) {
    case PlayerAnim::idle: {
      if (currDir != 0) {
        currAnim = PlayerAnim::run;
      } else if (vel.x != 0) {
        const float deaccelFactor = vel.x > 0 ? -1.5f : 1.5f;
        float deaccelVel = deaccelFactor * accel.x * dt;

        if (glm::abs(vel.x) < glm::abs(deaccelVel)) {
          vel.x = 0;
        } else {
          vel.x += deaccelVel;
        }
      }
      break;
    }
    case PlayerAnim::run: {
      if (currDir == 0) {
        currAnim = PlayerAnim::idle;
      }

      // NOTE: If `vel.x` and `dir` have different signs, their product is
      // less than zero.
      if (vel.x * dir < 0 && grounded) {
        currAnim = PlayerAnim::slide;
      }
      break;
    }
    case PlayerAnim::slide: {
      // NOTE: If `vel.x` and `currDir` have the same signs, their product is
      // greater than zero.
      if (vel.x * currDir > 0 && grounded) {
        currAnim = PlayerAnim::run;
      } else if (vel.x * currDir == 0 && grounded) {
        currAnim = PlayerAnim::idle;
      }
      break;
    }
  }

  vel += currDir * accel * dt;
  vel.x = glm::clamp(vel.x, -maxSpeedX, maxSpeedX);

  constexpr float gravity = 980.0f;
  if (!grounded) vel.y += gravity * dt;

  pos += vel * dt;
  collision(staticTiles, dynTiles, coins, collectedCoins, enemies);

  float camRuler = (SDLState::logicalWidth - w) / 2;

  if (!passedCamRuler && pos.x >= camRuler) passedCamRuler = true;
  if (passedCamRuler && pos.x >= camRuler) {
    cam.x = pos.x - camRuler;
  }

  // If the player is close to the top of the screen, then move the camera up
  // slightly so it does not look like the player is touching the ceiling or
  // going beyond it.
  constexpr float camYSmoothness = 5.0f;

  if (pos.y <= 10) {
    cam.y = glm::lerp(cam.y, -15.f, camYSmoothness * dt);
  } else {
    cam.y = glm::lerp(cam.y, 0.f, camYSmoothness * dt);
  }
}

void Player::collision(const std::vector<StaticTile>& staticTiles,
                       const std::vector<DynTile>& dynTiles,
                       std::vector<Coin>& coins, size_t& collectedCoins,
                       const std::vector<Enemy>& enemies) {
  SDL_FRect playerCollider{.x = pos.x + collider.x,
                           .y = pos.y + collider.y,
                           .w = collider.w,
                           .h = collider.h};
  SDL_FRect groundSensor{0, 0, 0, 0};
  SDL_FRect collidedRect{0, 0, 0, 0};
  SDL_FRect intersectionRect{0, 0, 0, 0};

  collided = false;
  bool foundGround = false;
  for (auto& staticTile : staticTiles) {
    collidedRect.x = staticTile.pos.x + staticTile.collider.x;
    collidedRect.y = staticTile.pos.y + staticTile.collider.y;
    collidedRect.w = staticTile.collider.w;
    collidedRect.h = staticTile.collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;
      if (intersectionRect.w < intersectionRect.h) {
        if (vel.x > 0) {
          pos.x -= intersectionRect.w;
        } else if (vel.x < 0) {
          pos.x += intersectionRect.w;
        }
        vel.x = 0;
      } else {
        if (vel.y > 0) {
          pos.y -= intersectionRect.h;
        } else if (vel.y < 0) {
          pos.y += intersectionRect.h;
        }
        vel.y = 0;
      }
    }

    // Recalculate playerCollider after the player has moved due to collision.
    playerCollider.x = pos.x + collider.x;
    playerCollider.y = pos.y + collider.y;

    groundSensor.x = playerCollider.x;
    groundSensor.y = playerCollider.y + playerCollider.h;
    groundSensor.w = playerCollider.w;
    groundSensor.h = 1;

    if (SDL_GetRectIntersectionFloat(&groundSensor, &collidedRect,
                                     &intersectionRect)) {
      foundGround = true;
    }
  }

  for (auto& dynTile : dynTiles) {
    collidedRect.x = dynTile.pos.x + dynTile.collider.x;
    collidedRect.y = dynTile.pos.y + dynTile.collider.y;
    collidedRect.w = dynTile.collider.w;
    collidedRect.h = dynTile.collider.h;

    // TODO:Implement collision behaviour of player with moving platform
    // tiles.
    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;
    }
  }

  for (size_t i = 0; i < coins.size(); i++) {
    collidedRect.x = coins[i].pos.x + coins[i].collider.x;
    collidedRect.y = coins[i].pos.y + coins[i].collider.y;
    collidedRect.w = coins[i].collider.w;
    collidedRect.h = coins[i].collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;
      coins[i] = coins.back();
      coins.pop_back();
      i--;
      collectedCoins++;
    }
  }

  for (auto& enemy : enemies) {
    collidedRect.x = enemy.pos.x + enemy.collider.x;
    collidedRect.y = enemy.pos.y + enemy.collider.y;
    collidedRect.w = enemy.collider.w;
    collidedRect.h = enemy.collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;
      death = true;
    }
  }

  if (grounded != foundGround) {
    grounded = foundGround;
    if (foundGround) {
      currAnim = PlayerAnim::run;
    }
  }
}
