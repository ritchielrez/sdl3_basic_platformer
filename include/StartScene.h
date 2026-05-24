#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "SDLState.h"
#include "Text.h"

namespace StartSceneBtns {
enum {
  PLAY,
  EXIT,
};
}  // namespace StartSceneBtns

class StartScene {
  const SDLState &sdlState;
  Text playText;
  Text exitText;
  uint8_t selectedBtn;

 public:
  bool shouldStartGame = false;
  bool shouldQuit = false;

  StartScene(const SDLState &sdlState)
      : sdlState(sdlState),
        playText(sdlState, "Play", glm::vec2(0)),
        exitText(sdlState, "Exit", glm::vec2(0)),
        selectedBtn(0) {
    int playTextWidth, playTextHeight, exitTextWidth, exitTextHeight;
    playText.getSize(&playTextWidth, &playTextHeight);
    exitText.getSize(&exitTextWidth, &exitTextHeight);

    // Center horizontally, space vertically in the middle
    playText.pos = {
        (SDLState::logicalWidth - static_cast<float>(playTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) - static_cast<float>(playTextHeight)};
    exitText.pos = {
        (SDLState::logicalWidth - static_cast<float>(exitTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) + 10.0f};
  }

  void update([[maybe_unused]] float dt) {
    float windowMouseX, windowMouseY;
    SDL_GetMouseState(&windowMouseX, &windowMouseY);
    float mouseX = windowMouseX;
    float mouseY = windowMouseY;
    SDL_RenderCoordinatesFromWindow(sdlState.renderer, windowMouseX,
                                    windowMouseY, &mouseX, &mouseY);

    // Check hover for Play
    int playTextWidth, playTextHeight;
    playText.getSize(&playTextWidth, &playTextHeight);
    if (mouseX >= playText.pos.x &&
        mouseX <= playText.pos.x + static_cast<float>(playTextWidth) &&
        mouseY >= playText.pos.y &&
        mouseY <= playText.pos.y + static_cast<float>(playTextHeight)) {
      selectedBtn = StartSceneBtns::PLAY;
    }

    // Check hover for Exit
    int exitTextWidth, exitTextHeight;
    exitText.getSize(&exitTextWidth, &exitTextHeight);
    if (mouseX >= exitText.pos.x &&
        mouseX <= exitText.pos.x + static_cast<float>(exitTextWidth) &&
        mouseY >= exitText.pos.y &&
        mouseY <= exitText.pos.y + static_cast<float>(exitTextHeight)) {
      selectedBtn = StartSceneBtns::EXIT;
    }

    // Update colors based on selection
    if (selectedBtn == StartSceneBtns::PLAY) {
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
          selectedBtn -= 1;
          break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          selectedBtn += 1;
          break;
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_SPACE:
          if (selectedBtn == StartSceneBtns::PLAY)
            shouldStartGame = true;
          else
            shouldQuit = true;
          break;
        default:
          break;
      }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        float windowMouseX = event.button.x;
        float windowMouseY = event.button.y;
        float mouseX = windowMouseX;
        float mouseY = windowMouseY;
        SDL_RenderCoordinatesFromWindow(sdlState.renderer, windowMouseX,
                                        windowMouseY, &mouseX, &mouseY);

        int playTextWidth, playTextHeight, exitTextWidth, exitTextHeight;
        playText.getSize(&playTextWidth, &playTextHeight);
        exitText.getSize(&exitTextWidth, &exitTextHeight);

        if (mouseX >= playText.pos.x &&
            mouseX <= playText.pos.x + static_cast<float>(playTextWidth) &&
            mouseY >= playText.pos.y &&
            mouseY <= playText.pos.y + static_cast<float>(playTextHeight)) {
          shouldStartGame = true;
        } else if (mouseX >= exitText.pos.x &&
                   mouseX <=
                       exitText.pos.x + static_cast<float>(exitTextWidth) &&
                   mouseY >= exitText.pos.y &&
                   mouseY <=
                       exitText.pos.y + static_cast<float>(exitTextHeight)) {
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
