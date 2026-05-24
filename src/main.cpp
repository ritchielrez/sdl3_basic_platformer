#include "Game.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  // Create `Game` object which initializes game entities.
  Game game{"sdl3_basic_platformer", SDL_WINDOW_RESIZABLE, nullptr};
  // Then run the game.
  game.run();
  return 0;
}
