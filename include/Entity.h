#pragma once

#include <glm/glm.hpp>

#include "Frames.h"
#include "SDLState.h"

struct Entity {
  glm::vec2 pos, vel;
  float w, h;
  SDL_FRect collider;
  int64_t currAnim;
  std::vector<Frames> anims;
  SDL_Texture *tex;
  float dir;

  Entity()
      : pos(glm::vec2(0)),
        vel(glm::vec2(0)),
        w(0),
        h(0),
        collider(0),
        currAnim(-1),
        tex(nullptr),
        dir(0) {}

  void draw(const SDLState &sdlState);
};
