#pragma once

#include <fmt/format.h>

#include <string>

#include "Entity.h"
#include "Map.h"

struct DynTile : public Entity {
  glm::vec2 origin;
  bool collided;

  DynTile() : origin(glm::vec2(0, 0)), collided(false) {}

  void update(float dt, const SDL_FRect& cam) {
    assert(dir != 0 && "Unreachable: DynTile cannot have a direction of 0");

    const SDL_FRect dynTileRect{.x = pos.x, .y = pos.y, .w = w, .h = h};
    if (SDL_HasRectIntersectionFloat(&dynTileRect, &cam)) {
      if (dir == 1 && pos.x >= origin.x + 2 * Map::TILE_SIZE) {
        dir = -1;
      } else if (dir == -1 && pos.x <= origin.x - 2 * Map::TILE_SIZE) {
        dir = 1;
      }
      vel.x = glm::abs(vel.x) * static_cast<float>(dir);
      pos += vel * dt;
    }
  }

  [[nodiscard]] std::string inspect() const {
    return fmt::format(
        "Position: ({}, {})\nVelocity: ({}, {})\nCollision: "
        "{}\n",
        pos.x, pos.y, vel.x, vel.y, collided);
  }
};
