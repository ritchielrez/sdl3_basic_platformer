#pragma once

#include <SDL3_ttf/SDL_textengine.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fmt/core.h>

#include <glm/glm.hpp>
#include <string_view>

#include "SDLState.h"

// Wraps SDL_ttf's text rendering into a reusable object. Each Text instance
// holds a GPU-side text texture that can be repositioned, recolored, and
// dynamically updated at runtime — essential for HUD displays like coin
// counters and enemy kill counts.
class Text {
  // Internal SDL_ttf text object. Created via TTF_CreateText using the global
  // text engine and font from SDLState. The text is re-rendered when its
  // string changes via assign() or prepend().
  TTF_Text *ttfText;

 public:
  // Screen-space position in logical coordinates (320×180 space).
  glm::vec2 pos;

  Text() : ttfText(nullptr), pos(glm::vec2(0, 0)) {}
  Text(const SDLState &sdlState, const std::string_view str,
       const glm::vec2 &pos)
      : pos(pos) {
    ttfText = TTF_CreateText(sdlState.textEngine, sdlState.font, str.data(),
                             str.size());
    TTF_SetTextColor(ttfText, 255, 255, 255, SDL_ALPHA_OPAQUE);
  }
  Text(const SDLState &sdlState, const std::string_view str,
       const glm::vec2 &&pos)
      : pos(pos) {
    ttfText = TTF_CreateText(sdlState.textEngine, sdlState.font, str.data(),
                             str.size());
    TTF_SetTextColor(ttfText, 255, 255, 255, SDL_ALPHA_OPAQUE);
  }

  Text(const Text &) = delete;
  Text &operator=(const Text &) = delete;
  Text(Text &&) noexcept = delete;
  Text &operator=(Text &&) noexcept = delete;

  // Replace the displayed text content. Used to update HUD values without
  // destroying and recreating the TTF_Text object.
  void assign(const std::string_view str) {
    TTF_SetTextString(ttfText, str.data(), str.size());
  }
  // Insert text at the beginning of the current string. Useful for prepending
  // labels or prefixes.
  void prepend(const std::string_view prefix) {
    TTF_InsertTextString(ttfText, 0, prefix.data(), prefix.size());
  }

  // Retrieve the rendered text dimensions in logical pixels. Needed for
  // positioning UI elements (e.g. centering text on screen).
  void getSize(int *w, int *h) const { TTF_GetTextSize(ttfText, w, h); }

  void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    TTF_SetTextColor(ttfText, r, g, b, a);
  }

  void setWrapWidth(int w) const {
    TTF_SetTextWrapWidth(ttfText, w);
  }

  // Draw the text at its screen-space position. This renders directly to the
  // current SDL renderer target using the text engine.
  void draw() const { TTF_DrawRendererText(ttfText, pos.x, pos.y); }

  ~Text() { TTF_DestroyText(ttfText); }
};
