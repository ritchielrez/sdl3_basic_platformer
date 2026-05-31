#pragma once

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <string>

#include "Entity.h"
#include "StaticTile.h"
#include "fmt/format.h"

// A basic enemy — the green slime. Patrols back and forth, reversing direction
// when it hits a wall. Defeated when the player stomps it from above.
struct Slime : public Entity {
  // True when the slime is overlapping a solid tile (wall collision).
  bool collided;

  Slime() : collided(false) {}

  // Update patrol movement and collision, but only if the slime is within the
  // camera's view (frustum culling). This saves CPU cycles for off-screen
  // enemies.
  void update(const std::vector<StaticTile>& staticTiles, float dt,
              const SDL_FRect& cam) {
    const SDL_FRect enemyRect{.x = pos.x, .y = pos.y, .w = w, .h = h};
    if (SDL_HasRectIntersectionFloat(&enemyRect, &cam)) {
      pos += vel * dt;
      collision(staticTiles);
    }
  }

  // AABB-vs-AABB collision with static tiles. When the slime hits a wall, it
  // reverses horizontal velocity and direction (so its sprite flips). Only
  // horizontal collision is resolved — slimes don't respond to floors/ceilings
  // (they have gravity disabled; they patrol on their spawn y-level).
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
        // Only resolve the collision along the axis of least overlap
        // (horizontal for wall-bouncing enemies). If intersection is wider
        // than tall, it's a vertical collision and we skip it.
        if (intersectionRect.w < intersectionRect.h) {
          if (vel.x > 0) {
            pos.x -= intersectionRect.w;
          } else if (vel.x < 0) {
            pos.x += intersectionRect.w;
          }
          vel.x *= -1.0f;
          dir *= -1;
        }

        enemyCollider.x = pos.x + collider.x;
      }
    }
  }

  [[nodiscard]] std::string inspect() const {
    return fmt::format(
        "Position: ({}, {})\nVelocity: ({}, {})\nCollision: "
        "{}\n",
        pos.x, pos.y, vel.x, vel.y, collided);
  }
};
