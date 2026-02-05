#include "Player.h"

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

  vel.y += 100 * dt;
  pos += vel * dt;
  collision(dt);
}

void Player::collision(float dt) {
  const SDL_FRect playerRect{.x = pos.x, .y = pos.y, .w = w, .h = h};
  SDL_FRect collidedRect{0};
  SDL_FRect intersectionRect{0};

  for (auto& staticTile : Game::staticTiles) {
    collidedRect.x = staticTile.pos.x;
    collidedRect.y = staticTile.pos.y;
    collidedRect.w = staticTile.w;
    collidedRect.h = staticTile.h;

    if (SDL_GetRectIntersectionFloat(&playerRect, &collidedRect,
                                     &intersectionRect)) {
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
  }

  for (auto& dynTile : Game::dynTiles) {
    collidedRect.x = dynTile.pos.x;
    collidedRect.y = dynTile.pos.y;
    collidedRect.w = dynTile.w;
    collidedRect.h = dynTile.h;

    // TODO:Implement collision behaviour of player with moving platform tiles.
    if (SDL_GetRectIntersectionFloat(&playerRect, &collidedRect,
                                     &intersectionRect)) {
    }
  }
}
