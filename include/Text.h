#pragma once

#include <SDL3_ttf/SDL_textengine.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fmt/core.h>

#include <glm/glm.hpp>
#include <string_view>

#include "SDLState.h"

class Text {
  TTF_Text *ttfText;
  glm::vec2 pos;

 public:
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

  // Delete copy constructor and copy assignment operator
  Text(const Text &) = delete;
  Text &operator=(const Text &) = delete;

  // Delete move constructor and move assignment operator
  Text(Text &&) noexcept = delete;
  Text &operator=(Text &&) noexcept = delete;

  void assign(const std::string_view str) {
    TTF_SetTextString(ttfText, str.data(), str.size());
  }

  void draw() const { TTF_DrawRendererText(ttfText, pos.x, pos.y); }

  ~Text() { TTF_DestroyText(ttfText); }
};
