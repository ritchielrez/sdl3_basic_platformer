#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "Colors.h"
#include "SDLState.h"
#include "Text.h"

// Button identifiers for the death screen. Mirrors the pattern from StartScene.
namespace DeathSceneBtns {
enum {
  RETRY,
  BACK_TO_START,
};
}  // namespace DeathSceneBtns

// Shown after the player dies. Displays "You Died!!!" and two options: retry
// the current level or return to the start menu. Supports keyboard and mouse
// input, same as StartScene.
class DeathScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  Text deathText;        // "You Died!!!" —  static header
  Text retryText;        // "Retry" button —  restarts the level
  Text backToStartText;  // "Back to Start Screen" button —  back to title
  // Currently highlighted button index (0 = Retry, 1 = Back to Start Scene). Modulo-wrapped
  // on up/down input.
  uint8_t selectedBtn;

 public:
  // Set to true when the player confirms "Retry". Triggers GameScene::reset()
  // and transitions back to SceneType::game.
  bool shouldRetry = false;
  // Set to true when the player confirms "Back to Start Screen". Resets the
  // game scene and transitions to SceneType::start.
  bool shouldBeBackToStart = false;

  DeathScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        deathText(sdlState, "You Died!!!", glm::vec2(0)),
        retryText(sdlState, "Retry", glm::vec2(0)),
        backToStartText(sdlState, "Back to Start", glm::vec2(0)),
        selectedBtn(0) {
    int deathTextWidth, deathTextHeight, retryTextWidth, retryTextHeight,
        backToStartTextWidth, backToStartTextHeight;
    deathText.getSize(&deathTextWidth, &deathTextHeight);
    retryText.getSize(&retryTextWidth, &retryTextHeight);
    backToStartText.getSize(&backToStartTextWidth, &backToStartTextHeight);

    // "You Died!!!" centered, 40px above the vertical center.
    // "Retry" centered at vertical center.
    // "Back to Start Screen" centered, 10px below vertical center.
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

    deathText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
  }

  void update([[maybe_unused]] float dt) {
    float windowMouseX, windowMouseY;
    SDL_GetMouseState(&windowMouseX, &windowMouseY);
    float mouseX = windowMouseX;
    float mouseY = windowMouseY;
    SDL_RenderCoordinatesFromWindow(sdlState.renderer, windowMouseX,
                                    windowMouseY, &mouseX, &mouseY);

    int retryTextWidth, retryTextHeight;
    retryText.getSize(&retryTextWidth, &retryTextHeight);
    if (mouseX >= retryText.pos.x &&
        mouseX <= retryText.pos.x + static_cast<float>(retryTextWidth) &&
        mouseY >= retryText.pos.y &&
        mouseY <= retryText.pos.y + static_cast<float>(retryTextHeight)) {
      selectedBtn = DeathSceneBtns::RETRY;
    }

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

    if (selectedBtn == DeathSceneBtns::RETRY) {
      retryText.setColor(Colors::hl.r, Colors::hl.g, Colors::hl.b,
                         Colors::hl.a);
      backToStartText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b,
                               Colors::fg.a);
    } else {
      retryText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b,
                         Colors::fg.a);
      backToStartText.setColor(Colors::hl.r, Colors::hl.g, Colors::hl.b,
                               Colors::hl.a);
    }
  }

  void handleEvent(const SDL_Event &event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.scancode) {
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
          selectedBtn -= 1;
          if (selectedBtn > DeathSceneBtns::BACK_TO_START)
            selectedBtn = DeathSceneBtns::RETRY;
          break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          selectedBtn += 1;
          if (selectedBtn > DeathSceneBtns::BACK_TO_START)
            selectedBtn = DeathSceneBtns::BACK_TO_START;
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
    // Draw the background image stretched to fill the entire screen.
    SDL_RenderTexture(sdlState.renderer, resourceManager.getDeathSceneBgTex(),
                      nullptr, nullptr);
    deathText.draw();
    retryText.draw();
    backToStartText.draw();
  }
};
