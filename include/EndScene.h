#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <cstdio>
#include <string>

#include "Colors.h"
#include "SDLState.h"
#include "Text.h"

namespace EndSceneBtns {
enum {
  RETRY,
  BACK_TO_START,
};
}

class EndScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  Text congratsText;
  Text pointsText;
  Text bestText;
  Text retryText;
  Text backToStartText;
  uint8_t selectedBtn;

  size_t currentPoints = 0;
  size_t bestPoints = 0;

 public:
  bool shouldRetry = false;
  bool shouldBeBackToStart = false;

  EndScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        congratsText(sdlState, "Congratulations!!!", glm::vec2(0)),
        pointsText(sdlState, "", glm::vec2(0)),
        bestText(sdlState, "", glm::vec2(0)),
        retryText(sdlState, "Retry", glm::vec2(0)),
        backToStartText(sdlState, "Back to Start", glm::vec2(0)),
        selectedBtn(EndSceneBtns::RETRY) {
    congratsText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b,
                          Colors::fg.a);
    pointsText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b,
                        Colors::fg.a);
    bestText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
  }

  void setPoints(size_t collectedCoins, size_t slainSlimes) {
    currentPoints = collectedCoins + slainSlimes * 3;

    bestPoints = 0;
    FILE *f = fopen("points.dat", "r");
    if (f) {
      fscanf(f, "%zu", &bestPoints);
      fclose(f);
    }

    if (currentPoints > bestPoints) {
      bestPoints = currentPoints;
      f = fopen("points.dat", "w");
      if (f) {
        fprintf(f, "%zu", bestPoints);
        fclose(f);
      }
    }

    pointsText.assign(
        fmt::format("Points: {} ({} coins + {} enemies x3)", currentPoints,
                     collectedCoins, slainSlimes));
    bestText.assign(fmt::format("Best: {}", bestPoints));

    int congratsTextWidth, congratsTextHeight, pointsTextWidth, pointsTextHeight,
        bestTextWidth, bestTextHeight, retryTextWidth, retryTextHeight,
        backToStartTextWidth, backToStartTextHeight;
    congratsText.getSize(&congratsTextWidth, &congratsTextHeight);
    pointsText.getSize(&pointsTextWidth, &pointsTextHeight);
    bestText.getSize(&bestTextWidth, &bestTextHeight);
    retryText.getSize(&retryTextWidth, &retryTextHeight);
    backToStartText.getSize(&backToStartTextWidth, &backToStartTextHeight);

    congratsText.pos = {
        (SDLState::logicalWidth - static_cast<float>(congratsTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) - 50.0f};
    pointsText.pos = {
        (SDLState::logicalWidth - static_cast<float>(pointsTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) - 30.0f};
    bestText.pos = {
        (SDLState::logicalWidth - static_cast<float>(bestTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) - 20.0f};
    retryText.pos = {
        (SDLState::logicalWidth - static_cast<float>(retryTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) -
            static_cast<float>(retryTextHeight) / 2.0f};
    backToStartText.pos = {
        (SDLState::logicalWidth - static_cast<float>(backToStartTextWidth)) /
            2.0f,
        (SDLState::logicalHeight / 2.0f) + 10.0f};
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
      selectedBtn = EndSceneBtns::RETRY;
    }

    int backToStartTextWidth, backToStartTextHeight;
    backToStartText.getSize(&backToStartTextWidth, &backToStartTextHeight);
    if (mouseX >= backToStartText.pos.x &&
        mouseX <=
            backToStartText.pos.x + static_cast<float>(backToStartTextWidth) &&
        mouseY >= backToStartText.pos.y &&
        mouseY <=
            backToStartText.pos.y + static_cast<float>(backToStartTextHeight)) {
      selectedBtn = EndSceneBtns::BACK_TO_START;
    }

    if (selectedBtn == EndSceneBtns::RETRY) {
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
          if (selectedBtn > EndSceneBtns::BACK_TO_START)
            selectedBtn = EndSceneBtns::RETRY;
          break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          selectedBtn += 1;
          if (selectedBtn > EndSceneBtns::BACK_TO_START)
            selectedBtn = EndSceneBtns::BACK_TO_START;
          break;
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_SPACE:
          if (selectedBtn == EndSceneBtns::RETRY)
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
    SDL_RenderTexture(sdlState.renderer, resourceManager.getEndSceneBgTex(),
                      nullptr, nullptr);
    congratsText.draw();
    pointsText.draw();
    bestText.draw();
    retryText.draw();
    backToStartText.draw();
  }
};
