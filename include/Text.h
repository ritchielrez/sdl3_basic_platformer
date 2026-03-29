#pragma once

#include <SDL3_ttf/SDL_textengine.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <glm/glm.hpp>

#include "SDLState.h"

class Text {
  TTF_Text *ttfText;
  std::string_view str;
  glm::vec2 pos;

 public:
  Text() : ttfText(nullptr), str(""), pos(glm::vec2(0, 0)) {}
  Text(const SDLState &sdlState, const std::string_view str,
       const glm::vec2 &pos)
      : str(str), pos(pos) {
    ttfText = TTF_CreateText(sdlState.textEngine, sdlState.font, str.data(),
                             str.size());
    TTF_SetTextColor(ttfText, 255, 255, 255, SDL_ALPHA_OPAQUE);
  }
  Text(const SDLState &sdlState, const std::string_view str,
       const glm::vec2 &&pos)
      : str(str), pos(pos) {
    ttfText = TTF_CreateText(sdlState.textEngine, sdlState.font, str.data(),
                             str.size());
    TTF_SetTextColor(ttfText, 255, 255, 255, SDL_ALPHA_OPAQUE);
  }
  // NOTE: We are forced to define an explictit `free()`, because static `Text`
  // instances may be created by `Game` class and they need to be freed manually
  // before `main()` ends. `SDL_ttf` requires a strict order of
  // deinitialization.
  // TODO: Implement `TextManager`, so static `Text` instances do not need to be
  // created.
  void free() const { TTF_DestroyText(ttfText); }

  TTF_Text *getTtfText() const noexcept { return ttfText; }
  std::string_view getStr() const noexcept { return str; }
  glm::vec2 getPos() const noexcept { return pos; }

  void draw() const noexcept { TTF_DrawRendererText(ttfText, pos.x, pos.y); }
};
