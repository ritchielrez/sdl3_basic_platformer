#pragma once

#include <fmt/format.h>

class DeathScene {
 public:
  void update([[maybe_unused]] float dt) { fmt::println("You died"); }
  void draw();
};
