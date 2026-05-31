#pragma once

#include <fmt/color.h>

#include "Game.h"

// Conditional debug logging utility. Messages are only printed when
// Game::debug is true (toggled via F1 in debug builds). Uses the fmt library
// for type-safe, Python-style format strings with terminal colorization.
namespace Log {
template <typename... T>
void debug(fmt::format_string<T...> fmt, T &&...args) {
  if (Game::debug) {
    // Yellow prefix + message for easy visual scanning in the terminal.
    fmt::print(fg(fmt::color::yellow), "[DEBUG] ");
    fmt::print(fg(fmt::color::yellow), fmt, std::forward<T>(args)...);
  }
}
}  // namespace Log
