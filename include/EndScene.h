#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <cstdio>

#include "Colors.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "Text.h"

// Button identifiers for the end/victory screen. Used to track which button
// is selected (highlighted) and which action to take on confirm.
namespace EndSceneBtns {
enum {
  RETRY,
  BACK_TO_START,
};
}  // namespace EndSceneBtns

// The victory screen shown after the player finishes the level. Displays the
// player's score (points from coins and slain enemies) alongside the all-time
// best score. Offers "Retry" and "Back to Start" options. Supports keyboard
// and mouse input, same as StartScene.
class EndScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  Text pointsText;       // "Points: ..." — current run score
  Text bestText;         // "Best: ..." — all-time high score
  Text retryText;        // "Retry" button — plays the level again
  Text backToStartText;  // "Back to Start" button — back to title
  // Currently highlighted button index (0 = Retry, 1 = Back to Start).
  uint8_t selectedBtn;

  size_t currentPoints = 0;
  size_t bestPoints = 0;

 public:
  // Set to true when the player confirms "Retry". Triggers GameScene::reset()
  // and transitions back to SceneType::game.
  bool shouldRetry = false;
  // Set to true when the player confirms "Back to Start". Resets the game
  // scene and transitions to SceneType::start.
  bool shouldBeBackToStart = false;

  EndScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        pointsText(sdlState, "", glm::vec2(0)),
        bestText(sdlState, "", glm::vec2(0)),
        retryText(sdlState, "Retry", glm::vec2(0)),
        backToStartText(sdlState, "Back to Start", glm::vec2(0)),
        selectedBtn(EndSceneBtns::RETRY) {
    // Set all text colors to the default foreground. Button colors switch
    // to the highlight color when selected (handled in update()).
    pointsText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
    bestText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
  }

  // Calculates the player's score (coins + enemies * 3), loads the saved
  // best score from disk, updates it if beaten, and positions all text
  // centered on screen. Called once before the scene is shown.
  void setPoints(size_t collectedCoins, size_t slainSlimes) {
    currentPoints = collectedCoins + slainSlimes * 3;

    // Load the previous best score from the binary file on disk.
    bestPoints = 0;
    FILE *f = fopen("points.dat", "r");
    if (f) {
      fscanf(f, "%zu", &bestPoints);
      fclose(f);
    }

    // If this run beat the record, save the new best score.
    if (currentPoints > bestPoints) {
      bestPoints = currentPoints;
      f = fopen("points.dat", "w");
      if (f) {
        fprintf(f, "%zu", bestPoints);
        fclose(f);
      }
    }

    // Build the score strings and position all text centered vertically,
    // stacked from above-center downward.
    pointsText.assign(fmt::format("Points: {} ({} coins + {} enemies x3)",
                                  currentPoints, collectedCoins, slainSlimes));
    bestText.assign(fmt::format("Best: {}", bestPoints));

    // Calculate widths and heights of all texts in the scene.
    int pointsTextWidth, pointsTextHeight, bestTextWidth, bestTextHeight,
        retryTextWidth, retryTextHeight, backToStartTextWidth,
        backToStartTextHeight;
    pointsText.getSize(&pointsTextWidth, &pointsTextHeight);
    bestText.getSize(&bestTextWidth, &bestTextHeight);
    retryText.getSize(&retryTextWidth, &retryTextHeight);
    backToStartText.getSize(&backToStartTextWidth, &backToStartTextHeight);

    // "Points: ..." centered, 30px above vertical center.
    // "Best: ..." centered, 20px above vertical center.
    // "Retry" centered at vertical center.
    // "Back to Start" centered, 10px below vertical center.
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

  // `dt` (delta time) is unused here because menu screens don't need
  // frame-rate-independent logic — they only react to input. The
  // [[maybe_unused]] attribute silences the compiler warning.
  void update([[maybe_unused]] float dt) {
    // Mouse coordinates come in window pixels (e.g. 1280x720) and need to be
    // converted to logical coordinates (320x180) for accurate hit-testing.
    // SDL_RenderCoordinatesFromWindow handles this transformation.
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

    // Highlight the selected button in yellow, dim the other to the default
    // foreground color. Provides visual feedback for which option is active.
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

  // Called whenever the player does something (a key press, a mouse click,
  // etc.). SDL wraps that info in an "event" and sends it here so the active
  // screen can react to what happened.
  void handleEvent(const SDL_Event &event) {
    // ---- KEYBOARD INPUT ----
    // First, check if the event is a key being pressed down.
    if (event.type == SDL_EVENT_KEY_DOWN) {
      // "Scancode" identifies which physical key was hit (not which character
      // it produces). We switch on it to run different code per key.
      switch (event.key.scancode) {
        // UP / W — highlight the previous button.
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
          selectedBtn -= 1;
          // If we moved past the first button, wrap back around to the last.
          if (selectedBtn > EndSceneBtns::BACK_TO_START)
            selectedBtn = EndSceneBtns::RETRY;
          break;
        // DOWN / S — highlight the next button.
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          selectedBtn += 1;
          // If we moved past the last button, stay on the last one.
          if (selectedBtn > EndSceneBtns::BACK_TO_START)
            selectedBtn = EndSceneBtns::BACK_TO_START;
          break;
        // ENTER / SPACE — activate whichever button is currently highlighted.
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
    // ---- MOUSE INPUT ----
    // Otherwise, check if the event is a mouse button click.
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      // Only react to the left mouse button.
      if (event.button.button == SDL_BUTTON_LEFT) {
        // The click coordinates come in real window pixels (say 1280x720),
        // but our game runs in a tiny 320x180 logical space. We need to
        // convert so the click lands on the right button.
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

        // Now check if the converted click falls inside any button's area.
        // A button is just a rectangle — we compare the mouse X and Y
        // against its left, right, top, and bottom edges.
        if (mouseX >= retryText.pos.x &&
            mouseX <= retryText.pos.x + static_cast<float>(retryTextWidth) &&
            mouseY >= retryText.pos.y &&
            mouseY <= retryText.pos.y + static_cast<float>(retryTextHeight)) {
          // Clicked "Retry" — restart the current level.
          shouldRetry = true;
        } else if (mouseX >= backToStartText.pos.x &&
                   mouseX <= backToStartText.pos.x +
                                 static_cast<float>(backToStartTextWidth) &&
                   mouseY >= backToStartText.pos.y &&
                   mouseY <= backToStartText.pos.y +
                                 static_cast<float>(backToStartTextHeight)) {
          // Clicked "Back to Start" — go to the title menu.
          shouldBeBackToStart = true;
        }
      }
    }
  }

  void draw() {
    // Draw the background image stretched to fill the entire screen.
    SDL_RenderTexture(sdlState.renderer, resourceManager.getEndSceneBgTex(),
                      nullptr, nullptr);
    // Draw all the texts.
    pointsText.draw();
    bestText.draw();
    retryText.draw();
    backToStartText.draw();
  }
};
