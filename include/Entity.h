#pragma once

#include <glm/glm.hpp>

#include "Frames.h"
#include "SDLState.h"

// Base entity struct shared by the player, enemies, tiles, and coins.
// Provides position, velocity, collision, and animation data — the minimal
// components needed for a 2D platformer actor.
struct Entity {
  // World-space position and velocity in pixels. Used for Euler integration:
  //   pos += vel * dt
  glm::vec2 pos, vel;
  // Entity dimensions (width and height) in pixels.
  float w, h;
  // Axis-aligned bounding box (AABB) for collision detection. Stored as an
  // offset relative to `pos` so the entity can have a hitbox smaller than its
  // visual sprite (common in platformers for fair gameplay).
  SDL_FRect collider;
  // Index into `anims` for the currently active animation. -1 means no
  // animation is playing.
  int64_t currAnim;
  // All animations this entity can play (idle, run, jump, etc.). Each entry is
  // a `Frames` object holding frame timings and sprite-sheet UVs.
  std::vector<Frames> anims;
  // The sprite-sheet texture loaded into GPU VRAM via SDL.
  SDL_Texture *tex;
  // Facing direction: 1 = right, -1 = left. Used to flip the sprite when
  // the entity changes direction.
  int16_t dir;

  // Zero-initialize all entity fields. Position and velocity start at the
  // origin, no animation is active, and the texture pointer is null.
  Entity()
      : pos(glm::vec2(0)),
        vel(glm::vec2(0)),
        w(0),
        h(0),
        collider(0),
        currAnim(-1),
        tex(nullptr),
        dir(0) {}

  // Draw the entity's current animation frame at its world position,
  // adjusted by the camera offset for scrolling.
  void draw(const SDLState &sdlState, const SDL_FRect &cam);
};
