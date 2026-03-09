#include "Player.h"

#include <SDL3/SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include "Game.h"
#include "Log.h"
#include "SDLState.h"

void Player::update(const SDLState& sdlState, float dt) {
  float currDir = 0;
  if (sdlState.keys[SDL_SCANCODE_A]) {
    currDir -= 1;
  }
  if (sdlState.keys[SDL_SCANCODE_D]) {
    currDir += 1;
  }
  if (grounded && sdlState.keys[SDL_SCANCODE_SPACE]) {
    vel.y = jumpVel;
    currAnim = PlayerAnim::jump;
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
          const float deaccelFactor = vel.x > 0 ? -1.5f : 1.5f;
          float deaccelVel = deaccelFactor * accel.x * dt;

          if (glm::abs(vel.x) < glm::abs(deaccelVel)) {
            vel.x = 0;
          } else {
            vel.x += deaccelVel;
          }
        }
      }
      break;
    }
    case PlayerAnim::run: {
      if (!currDir) {
        currAnim = PlayerAnim::idle;
      }

      if (vel.x * dir < 0 && grounded) {
        currAnim = PlayerAnim::slide;
      }
      break;
    }
    case PlayerAnim::slide: {
      if (vel.x * dir >= 0 || !grounded) {
        currAnim = PlayerAnim::run;
      }
      break;
    }
  }

  vel += currDir * accel * dt;
  vel.x = glm::clamp(vel.x, -maxSpeedX, maxSpeedX);

  constexpr float gravVel = 150.0f;
  if (!grounded) vel.y += gravVel * dt;

  pos += vel * dt;
  collision(dt);

  float rightRuler = Game::cam.w * 0.75f;
  float leftRuler = Game::cam.w * 0.25f;
  float playerScreenX = pos.x - Game::cam.x;

  if (currDir == 1 && playerScreenX >= leftRuler) {
    Game::cam.x = pos.x - leftRuler;
  } else if (currDir == -1 && playerScreenX <= rightRuler) {
    Game::cam.x = pos.x - rightRuler;
  }

  if (Game::debug) {
    SDL_SetRenderDrawColor(sdlState.renderer, 0, 255, 0, 128);
    SDL_RenderLine(sdlState.renderer, Game::cam.w / 4, 0, Game::cam.w / 4,
                   Game::cam.h);
    SDL_RenderLine(sdlState.renderer, Game::cam.w * 3 / 4, 0,
                   Game::cam.w * 3 / 4, Game::cam.h);
  }
}

void Player::collision(float dt) {
  SDL_FRect playerCollider{.x = pos.x + collider.x,
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
      currAnim = PlayerAnim::run;
    }
  }
}
