#pragma once

#include "SDLState.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_ttf/SDL_textengine.h>

#include <glm/glm.hpp>

class Text {
  TTF_Text *text;
  glm::vec2 pos;
public:
  Text(const SDLState &sdlState, const std::string_view textStr, const glm::vec2& pos) : pos(pos) {
    text = TTF_CreateText(sdlState.textEngine, sdlState.font, textStr.data(), textStr.size());
    TTF_SetTextColor(text, 255, 255, 255, SDL_ALPHA_OPAQUE);
  }
  Text(const SDLState &sdlState, const std::string_view textStr, const glm::vec2&& pos) : pos(pos) {
    text = TTF_CreateText(sdlState.textEngine, sdlState.font, textStr.data(), textStr.size());
    TTF_SetTextColor(text, 255, 255, 255, SDL_ALPHA_OPAQUE);
  }

  void draw() const { TTF_DrawRendererText(text, pos.x, pos.y); }

  ~Text() { TTF_DestroyText(text); }
};
