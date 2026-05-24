#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "SDLState.h"
#include "Text.h"

namespace DeathSceneBtns {
enum {
  RETRY,
  BACK_TO_START,
};
}  // namespace DeathSceneBtns

class DeathScene {
  const SDLState &sdlState;
  Text deathText;
  Text retryText;
  Text backToStartText;
  uint8_t selectedBtn;

 public:
  bool shouldRetry = false;
  bool shouldBeBackToStart = false;

  DeathScene(const SDLState &sdlState)
      : sdlState(sdlState),
        deathText(sdlState, "You Died!!!", glm::vec2(0)),
        retryText(sdlState, "Retry", glm::vec2(0)),
        backToStartText(sdlState, "Back to Start Screen", glm::vec2(0)),
        selectedBtn(0) {
    int deathTextWidth, deathTextHeight, retryTextWidth, retryTextHeight,
        backToStartTextWidth, backToStartTextHeight;
    deathText.getSize(&deathTextWidth, &deathTextHeight);
    retryText.getSize(&retryTextWidth, &retryTextHeight);
    backToStartText.getSize(&backToStartTextWidth, &backToStartTextHeight);

    // Center horizontally, space vertically in the middle
    deathText.pos = {
        (SDLState::logicalWidth - static_cast<float>(deathTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) - 40.0f};
    retryText.pos = {
        (SDLState::logicalWidth - static_cast<float>(retryTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) -
            static_cast<float>(retryTextHeight) / 2.0f};
    backToStartText.pos = {
        (SDLState::logicalWidth - static_cast<float>(backToStartTextWidth)) /
            2.0f,
        (SDLState::logicalHeight / 2.0f) + 10.0f};

    deathText.setColor(255, 255, 255);  // White
  }

  void update([[maybe_unused]] float dt) {
    float windowMouseX, windowMouseY;
    SDL_GetMouseState(&windowMouseX, &windowMouseY);
    float mouseX = windowMouseX;
    float mouseY = windowMouseY;
    SDL_RenderCoordinatesFromWindow(sdlState.renderer, windowMouseX,
                                    windowMouseY, &mouseX, &mouseY);

    // Check hover for Retry
    int retryTextWidth, retryTextHeight;
    retryText.getSize(&retryTextWidth, &retryTextHeight);
    if (mouseX >= retryText.pos.x &&
        mouseX <= retryText.pos.x + static_cast<float>(retryTextWidth) &&
        mouseY >= retryText.pos.y &&
        mouseY <= retryText.pos.y + static_cast<float>(retryTextHeight)) {
      selectedBtn = DeathSceneBtns::RETRY;
    }

    // Check hover for Back to Start Screen
    int backToStartTextWidth, backToStartTextHeight;
    backToStartText.getSize(&backToStartTextWidth, &backToStartTextHeight);
    if (mouseX >= backToStartText.pos.x &&
        mouseX <=
            backToStartText.pos.x + static_cast<float>(backToStartTextWidth) &&
        mouseY >= backToStartText.pos.y &&
        mouseY <=
            backToStartText.pos.y + static_cast<float>(backToStartTextHeight)) {
      selectedBtn = DeathSceneBtns::BACK_TO_START;
    }

    // Update colors based on selection
    if (selectedBtn == DeathSceneBtns::RETRY) {
      retryText.setColor(255, 255, 0);          // Yellow
      backToStartText.setColor(255, 255, 255);  // White
    } else {
      retryText.setColor(255, 255, 255);
      backToStartText.setColor(255, 255, 0);
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
          if (selectedBtn == DeathSceneBtns::RETRY)
            shouldRetry = true;
          else
            shouldBeBackToStart = true;
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

        int retryTextWidth, retryTextHeight, backToStartTextWidth,
            backToStartTextHeight;
        retryText.getSize(&retryTextWidth, &retryTextHeight);
        backToStartText.getSize(&backToStartTextWidth, &backToStartTextHeight);

        if (mouseX >= retryText.pos.x &&
            mouseX <= retryText.pos.x + static_cast<float>(retryTextWidth) &&
            mouseY >= retryText.pos.y &&
            mouseY <= retryText.pos.y + static_cast<float>(retryTextHeight)) {
          shouldRetry = true;
        } else if (mouseX >= backToStartText.pos.x &&
                   mouseX <= backToStartText.pos.x +
                                 static_cast<float>(backToStartTextWidth) &&
                   mouseY >= backToStartText.pos.y &&
                   mouseY <= backToStartText.pos.y +
                                 static_cast<float>(backToStartTextHeight)) {
          shouldBeBackToStart = true;
        }
      }
    }
  }

  void draw() {
    deathText.draw();
    retryText.draw();
    backToStartText.draw();
  }
};
