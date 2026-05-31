#pragma once

// Simple RGBA color representation used throughout the game for UI text. Each
// channel is an 8-bit unsigned integer (0–255).
struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

// Central registry of named game colors. Keeping them in one place makes it
// easy to tweak the game's palette without hunting through every file.
struct Colors {
  // Default foreground color: dark brown, fully opaque. Used for UI text.
  static constexpr Color fg{.r = 50, .g = 40, .b = 15, .a = 255};
  static constexpr Color hl{.r = 124, .g = 98, .b = 22, .a = 255};
};
