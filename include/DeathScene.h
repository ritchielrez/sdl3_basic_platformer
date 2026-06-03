#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "Colors.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "Text.h"

// Button identifiers for the death screen. Used to track which button is
// selected (highlighted) and which action to take on confirm.
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
  Text backToStartText;  // "Back to Start" button —  back to title
  // Currently highlighted button index (0 = Retry, 1 = Back to Start).
  uint8_t selectedBtn;

 public:
  // Set to true when the player confirms "Retry". Triggers GameScene::reset()
  // and transitions back to SceneType::game.
  bool shouldRetry = false;
  // Set to true when the player confirms "Back to Start". Resets the game
  // scene and transitions to SceneType::start.
  bool shouldBeBackToStart = false;

  // Construct the death screen. Creates "You Died!!!", "Retry", and
  // "Back to Start" text objects, positions them centered on screen,
  // and defaults to the Retry button being selected.
  DeathScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        deathText(sdlState, "You Died!!!", glm::vec2(0)),
        retryText(sdlState, "Retry", glm::vec2(0)),
        backToStartText(sdlState, "Back to Start", glm::vec2(0)),
        selectedBtn(DeathSceneBtns::RETRY) {
    // Calculate the widths and heights of the texts.
    int deathTextWidth, deathTextHeight, retryTextWidth, retryTextHeight,
        backToStartTextWidth, backToStartTextHeight;
    deathText.getSize(&deathTextWidth, &deathTextHeight);
    retryText.getSize(&retryTextWidth, &retryTextHeight);
    backToStartText.getSize(&backToStartTextWidth, &backToStartTextHeight);

    // "You Died!!!" centered, 40px above the vertical center.
    // "Retry" centered at vertical center.
    // "Back to Start" centered, 10px below vertical center.
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

    // Set the deathText color to be the default foreground color.
    deathText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
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

    // Calculate widths and heights of all texts. These calculations help us 
    // detect what button the user is hovering on. Change `selectedBtn` based
    // on the button the user is hovering on.
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

    // Highlight the selected button in highlighting foreground color set in Colors.h, 
    // dim the other to the default foreground color. Provides visual feedback for which 
    // option is active.
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
          if (selectedBtn > DeathSceneBtns::BACK_TO_START)
            selectedBtn = DeathSceneBtns::RETRY;
          break;
        // DOWN / S — highlight the next button.
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          selectedBtn += 1;
          // If we moved past the last button, stay on the last one.
          if (selectedBtn > DeathSceneBtns::BACK_TO_START)
            selectedBtn = DeathSceneBtns::BACK_TO_START;
          break;
        // ENTER / SPACE — activate whichever button is currently highlighted.
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

  // Draw the background image and all three text elements (death header,
  // retry button, back-to-start button).
  void draw() {
    // Draw the background image stretched to fill the entire screen.
    SDL_RenderTexture(sdlState.renderer, resourceManager.getDeathSceneBgTex(),
                      nullptr, nullptr);
    deathText.draw();
    retryText.draw();
    backToStartText.draw();
  }
};
