#pragma once

#include <fmt/color.h>

#include "Game.h"

namespace Log {
// TODO: Maybe move this to UI, so instead of logging to console it will log in
// the in game UI.
template <typename... T>
void debug(fmt::format_string<T...> fmt, T &&...args) {
  if (Game::debug) {
    fmt::print(fg(fmt::color::yellow), "[DEBUG] ");
    fmt::print(fg(fmt::color::yellow), fmt, std::forward<T>(args)...);
  }
}
}  // namespace Log
