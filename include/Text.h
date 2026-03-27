#pragma once

#include "SDLState.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_ttf/SDL_textengine.h>

#include <glm/glm.hpp>

class Text {
  TTF_Text *ttfText;
  std::string_view str;
  glm::vec2 pos;
public:
  Text() : ttfText(nullptr), str(""), pos(glm::vec2(0, 0)) {}
  Text(const SDLState &sdlState, const std::string_view str, const glm::vec2& pos) : str(str), pos(pos) {
    ttfText = TTF_CreateText(sdlState.textEngine, sdlState.font, str.data(), str.size());
    TTF_SetTextColor(ttfText, 255, 255, 255, SDL_ALPHA_OPAQUE);
  }
  Text(const SDLState &sdlState, const std::string_view str, const glm::vec2&& pos) : str(str), pos(pos) {
    ttfText = TTF_CreateText(sdlState.textEngine, sdlState.font, str.data(), str.size());
    TTF_SetTextColor(ttfText, 255, 255, 255, SDL_ALPHA_OPAQUE);
  }

  Text &operator=(const Text &text) {
    ttfText = text.getTtfText();
    str = text.getStr();
    pos = text.getPos();
    return *this;
  }

  TTF_Text *getTtfText () const { return ttfText; }
  std::string_view getStr() const { return str; }
  glm::vec2 getPos() const { return pos; }

  void draw() const { TTF_DrawRendererText(ttfText, pos.x, pos.y); }

  ~Text() { TTF_DestroyText(ttfText); }
};
