// SDL3 is a library that can handle audio, input and graphics
// accross various platforms. One of the biggest users of SDL is Valve, as
// many of their games such as Team Fortress 2 use SDL for some parts of the
// games.
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// A builtin error handing library which provided `assert()`. This function
// aborts the program if the user-specified condition is not true. May be
// disabled for release builds
#include <cassert>
// glm is a C++ math library offering important math objects such as vectors,
// matrices etc.
#include <glm/glm.hpp>

#include "Game.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  // Create `Game` object which initializes game entities.
  Game game{"sdl3_basic_platformer", SDL_WINDOW_RESIZABLE, nullptr};
  // Then run the game.
  game.run();
  return 0;
}
