#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "SDLState.h"
#include "Text.h"

namespace StartSceneButtons {
enum {
  PLAY,
  EXIT,
};
}  // namespace StartSceneButtons

class StartScene {
  const SDLState &sdlState;
  Text playText;
  Text exitText;
  uint8_t selectedButton = StartSceneButtons::PLAY;

 public:
  bool shouldStartGame = false;
  bool shouldQuit = false;

  StartScene(const SDLState &sdlState)
      : sdlState(sdlState),
        playText(sdlState, "Play", glm::vec2(0)),
        exitText(sdlState, "Exit", glm::vec2(0)) {
    int pw, ph, ew, eh;
    playText.getSize(&pw, &ph);
    exitText.getSize(&ew, &eh);

    // Center horizontally, space vertically in the middle
    playText.pos = {(SDLState::logicalWidth - pw) / 2.0f,
                    (SDLState::logicalHeight / 2.0f) - ph};
    exitText.pos = {(SDLState::logicalWidth - ew) / 2.0f,
                    (SDLState::logicalHeight / 2.0f) + 10.0f};
  }

  void update(float dt) {
    float mx, my;
    SDL_GetMouseState(&mx, &my);

    // Convert screen coordinates to logical coordinates for hit testing
    // However, SDL3's logical presentation usually handles this if we use the
    // right event values. For manual GetMouseState, we might need to scale, but
    // SDL_GetMouseState in SDL3 often returns values in the logical coordinate
    // space if configured. Let's assume logical coordinates for now as SDLState
    // sets up logical presentation.

    // Check hover for Play
    int pw, ph;
    playText.getSize(&pw, &ph);
    if (mx >= playText.pos.x && mx <= playText.pos.x + pw &&
        my >= playText.pos.y && my <= playText.pos.y + ph) {
      selectedButton = 0;
    }

    // Check hover for Exit
    int ew, eh;
    exitText.getSize(&ew, &eh);
    if (mx >= exitText.pos.x && mx <= exitText.pos.x + ew &&
        my >= exitText.pos.y && my <= exitText.pos.y + eh) {
      selectedButton = 1;
    }

    // Update colors based on selection
    if (selectedButton == 0) {
      playText.setColor(255, 255, 0);    // Yellow
      exitText.setColor(255, 255, 255);  // White
    } else {
      playText.setColor(255, 255, 255);
      exitText.setColor(255, 255, 0);
    }
  }

  void handleEvent(const SDL_Event &event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.scancode) {
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
          selectedButton = 0;
          break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          selectedButton = 1;
          break;
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_SPACE:
          if (selectedButton == 0)
            shouldStartGame = true;
          else
            shouldQuit = true;
          break;
        default:
          break;
      }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        float mx = event.button.x;
        float my = event.button.y;

        int pw, ph, ew, eh;
        playText.getSize(&pw, &ph);
        exitText.getSize(&ew, &eh);

        if (mx >= playText.pos.x && mx <= playText.pos.x + pw &&
            my >= playText.pos.y && my <= playText.pos.y + ph) {
          shouldStartGame = true;
        } else if (mx >= exitText.pos.x && mx <= exitText.pos.x + ew &&
                   my >= exitText.pos.y && my <= exitText.pos.y + eh) {
          shouldQuit = true;
        }
      }
    }
  }

  void draw() {
    playText.draw();
    exitText.draw();
  }
};
