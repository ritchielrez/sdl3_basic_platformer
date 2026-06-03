#pragma once

#include <fmt/format.h>

#include <string>

#include "Entity.h"
#include "Map.h"

// A moving platform tile that patrols horizontally between two points. The
// platform oscillates ±2 tiles from its spawn origin, carrying the player
// along when they stand on it (handled in Player::collision). Only updates
// if it is within the camera's view.
struct DynTile : public Entity {
  // The tile's spawn position in the level. Used as the center point of the
  // patrol route.
  glm::vec2 origin;
  bool collided;

  // Default constructor.  Initialises the patrol origin to (0, 0) and marks
  // the tile as not yet collided with anything.
  DynTile() : origin(glm::vec2(0, 0)), collided(false) {}

  // Moves the platform along its patrol route.  Only updates if the tile is
  // visible within the camera's rectangle.  The platform oscillates ±2 tiles
  // (32 px) from its spawn origin — it reverses direction when either
  // boundary is reached.
  void update(float dt, const SDL_FRect& cam) {
    assert(dir != 0 && "Unreachable: DynTile cannot have a direction of 0");

    const SDL_FRect dynTileRect{.x = pos.x, .y = pos.y, .w = w, .h = h};
    if (SDL_HasRectIntersectionFloat(&dynTileRect, &cam)) {
      // Reverse direction when reaching the patrol boundary (±2 tiles from
      // origin, where each tile is 16px).
      if (dir == 1 && pos.x >= origin.x + 2 * Map::TILE_SIZE) {
        dir = -1;
      } else if (dir == -1 && pos.x <= origin.x - 2 * Map::TILE_SIZE) {
        dir = 1;
      }
      vel.x = glm::abs(vel.x) * static_cast<float>(dir);
      pos += vel * dt;
    }
  }

  // Returns a human-readable string of the tile's current position, velocity,
  // and collision state.  Used by the debug overlay (DebugUI) to display live
  // tile information.
  [[nodiscard]] std::string inspect() const {
    return fmt::format(
        "Position: ({}, {})\nVelocity: ({}, {})\nCollision: "
        "{}\n",
        pos.x, pos.y, vel.x, vel.y, collided);
  }
};
