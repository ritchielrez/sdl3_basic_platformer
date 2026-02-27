#include "Player.h"

#include <SDL3/SDL.h>
#include <imgui.h>

#include "Game.h"
#include "Log.h"
#include "SDLState.h"
#include "UI.h"

void Player::update(const SDLState& sdlState, float dt) {
  float currDir = 0;
  if (sdlState.keys[SDL_SCANCODE_A]) {
    currDir -= 1;
  }
  if (sdlState.keys[SDL_SCANCODE_D]) {
    currDir += 1;
  }
  if (currDir) {
    dir = currDir;
  }

  switch (currAnim) {
    case PlayerAnim::idle: {
      if (currDir) {
        currAnim = PlayerAnim::run;
      } else {
        if (vel.x) {
          const float factor = vel.x > 0 ? -1.5f : 1.5f;
          float amount = factor * accel.x * dt;

          if (glm::abs(vel.x) < glm::abs(amount)) {
            vel.x = 0;
          } else {
            vel.x += amount;
          }
        }
      }
      break;
    }
    case PlayerAnim::run: {
      if (!currDir) {
        currAnim = PlayerAnim::idle;
      }
      break;
    }
  }

  vel += currDir * accel * dt;
  if (glm::abs(vel.x) > maxSpeedX) {
    vel.x = currDir * maxSpeedX;
  }

  vel.y += 150 * dt;
  pos += vel * dt;
  collision(dt);
}

void Player::collision(float dt) {
  const SDL_FRect playerCollider{.x = pos.x + collider.x,
                                 .y = pos.y + collider.y,
                                 .w = collider.w,
                                 .h = collider.h};
  SDL_FRect groundSensor{0};
  SDL_FRect collidedRect{0};
  SDL_FRect intersectionRect{0};

  collided = false;
  bool foundGround = false;
  for (auto& staticTile : Game::staticTiles) {
    collidedRect.x = staticTile.pos.x;
    collidedRect.y = staticTile.pos.y;
    collidedRect.w = staticTile.w;
    collidedRect.h = staticTile.h;

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

    groundSensor.x = playerCollider.x;
    groundSensor.y = playerCollider.y + playerCollider.h;
    groundSensor.w = playerCollider.w;
    groundSensor.h = 1;

    if (SDL_GetRectIntersectionFloat(&groundSensor, &collidedRect,
                                     &intersectionRect)) {
      foundGround = true;
    }
  }

  for (auto& dynTile : Game::dynTiles) {
    collidedRect.x = dynTile.pos.x;
    collidedRect.y = dynTile.pos.y;
    collidedRect.w = dynTile.w;
    collidedRect.h = dynTile.h;

    // TODO:Implement collision behaviour of player with moving platform tiles.
    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;
    }
  }

  if (grounded != foundGround) {
    grounded = foundGround;
    if (foundGround) {
      Log::debug("Player found ground: true\n");
      currAnim = PlayerAnim::run;
    }
  }
}

void Player::handleKeyInput(const SDLState& sdlState, SDL_Scancode key,
                            bool keyDown) {
  switch (currAnim) {
    case PlayerAnim::idle: {
      if (key == SDL_SCANCODE_SPACE && keyDown) {
        currAnim = PlayerAnim::jump;
        vel.y += jumpAccel;
      }
      break;
    }
    case PlayerAnim::run: {
      if (key == SDL_SCANCODE_SPACE && keyDown) {
        currAnim = PlayerAnim::jump;
        vel.y += jumpAccel;
      }
      break;
    }
  }
}
