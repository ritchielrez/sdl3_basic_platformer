#pragma once

#include "Entity.h"

struct DynTile : public Entity {
  glm::vec2 accel;

  DynTile() : accel(glm::vec2(0)) {}
};
