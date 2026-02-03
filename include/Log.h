#pragma once

#include <fmt/color.h>

namespace Log {
template <typename... T>
void debug(fmt::format_string<T...> fmt, T &&...args) {
#ifdef DEBUG
  fmt::print(fg(fmt::color::yellow), "[DEBUG] ");
  fmt::print(fg(fmt::color::yellow), fmt, std::forward<T>(args)...);
#endif
}
}  // namespace Log
