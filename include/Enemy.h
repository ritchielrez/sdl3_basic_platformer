#pragma once

#include <glm/glm.hpp>

#include "Entity.h"
#include "StaticTile.h"

struct Enemy : public Entity {
  bool collided;

  Enemy() : collided(false) {}

  void update(const std::vector<StaticTile>& staticTiles, float dt,
              const SDL_FRect& cam) {
    if (pos.x > cam.x && pos.x < cam.x + cam.w && pos.y > cam.y &&
        pos.y < cam.y + cam.h) {
      (void)dt;
      // pos += vel * dt;
      collision(staticTiles);
    }
  }

  void collision(const std::vector<StaticTile>& staticTiles) {
    SDL_FRect enemyCollider{.x = pos.x + collider.x,
                            .y = pos.y + collider.y,
                            .w = collider.w,
                            .h = collider.h};
    SDL_FRect collidedRect{0, 0, 0, 0};
    SDL_FRect intersectionRect{0, 0, 0, 0};

    collided = false;
    for (auto& staticTile : staticTiles) {
      collidedRect.x = staticTile.pos.x + staticTile.collider.x;
      collidedRect.y = staticTile.pos.y + staticTile.collider.y;
      collidedRect.w = staticTile.collider.w;
      collidedRect.h = staticTile.collider.h;

      if (SDL_GetRectIntersectionFloat(&enemyCollider, &collidedRect,
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
    }
  }
};
