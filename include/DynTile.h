#pragma once

#include <fmt/format.h>

#include <string>

#include "Entity.h"
#include "Map.h"
#include "StaticTile.h"

struct DynTile : public Entity {
  glm::vec2 origin;
  bool collided;

  DynTile() : origin(glm::vec2(0, 0)), collided(false) {}

  void update(const std::vector<StaticTile>& staticTiles, float dt,
              const SDL_FRect& cam) {
    assert(dir != 0 && "Unreachable: DynTile cannot have a direction of 0");

    const SDL_FRect dynTileRect{.x = pos.x, .y = pos.y, .w = w, .h = h};
    if (SDL_HasRectIntersectionFloat(&dynTileRect, &cam)) {
      if (dir == 1 && pos.x >= origin.x + 2 * Map::TILE_SIZE) {
        dir = -1;
      } else if (dir == -1 && pos.x <= origin.x - 2 * Map::TILE_SIZE) {
        dir = 1;
      }
      vel.x = glm::abs(vel.x) * dir;
      pos += vel * dt;
      // collision(staticTiles);
    }
  }

  // void collision(const std::vector<StaticTile>& staticTiles) {
  //   SDL_FRect enemyCollider{.x = pos.x + collider.x,
  //                           .y = pos.y + collider.y,
  //                           .w = collider.w,
  //                           .h = collider.h};
  //   SDL_FRect collidedRect{0, 0, 0, 0};
  //   SDL_FRect intersectionRect{0, 0, 0, 0};
  //
  //   collided = false;
  //   for (auto& staticTile : staticTiles) {
  //     collidedRect.x = staticTile.pos.x + staticTile.collider.x;
  //     collidedRect.y = staticTile.pos.y + staticTile.collider.y;
  //     collidedRect.w = staticTile.collider.w;
  //     collidedRect.h = staticTile.collider.h;
  //
  //     if (SDL_GetRectIntersectionFloat(&enemyCollider, &collidedRect,
  //                                      &intersectionRect)) {
  //       collided = true;
  //       if (intersectionRect.w < intersectionRect.h) {
  //         if (vel.x > 0) {
  //           pos.x -= intersectionRect.w;
  //         } else if (vel.x < 0) {
  //           pos.x += intersectionRect.w;
  //         }
  //         vel.x *= -1.0f;
  //         dir *= -1;
  //       }
  //
  //       // Recalculate enemyCollider after the enemy has moved due to collision.
  //       enemyCollider.x = pos.x + collider.x;
  //     }
  //   }
  // }

  [[nodiscard]] std::string inspect() const {
    return fmt::format(
        "Position: ({}, {})\nVelocity: ({}, {})\nCollision: "
        "{}\n",
        pos.x, pos.y, vel.x, vel.y, collided);
  }
};
