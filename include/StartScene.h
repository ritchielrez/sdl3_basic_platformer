#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "Colors.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "Text.h"

// Button identifiers for the start menu. Used to track which button is
// selected (highlighted) and which action to take on confirm.
namespace StartSceneBtns {
enum {
  PLAY,
  EXIT,
};
}  // namespace StartSceneBtns

// The title / main menu scene. Displays a background image, a "Play" button,
// and an "Exit" button. Supports both keyboard navigation (WASD / arrows +
// Enter/Space) and mouse (hover + click).
class StartScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  Text playText;
  Text exitText;
  // Currently highlighted button index (0 = Play, 1 = Exit). Modulo-wrapped
  // on up/down input.
  uint8_t selectedBtn;

 public:
  // Set to true when the player confirms "Play". Read by SceneManager to
  // transition to the game scene.
  bool shouldStartGame = false;
  // Set to true when the player confirms "Exit" or clicks the window close
  // button. Read by Game::run to break the main loop.
  bool shouldQuit = false;

  StartScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        playText(sdlState, "Play", glm::vec2(0)),
        exitText(sdlState, "Exit", glm::vec2(0)),
        selectedBtn(0) {
    int playTextWidth, playTextHeight, exitTextWidth, exitTextHeight;
    playText.getSize(&playTextWidth, &playTextHeight);
    exitText.getSize(&exitTextWidth, &exitTextHeight);

    // Position buttons centered horizontally, stacked vertically around the
    // middle of the logical screen (320×180).
    playText.pos = {
        (SDLState::logicalWidth - static_cast<float>(playTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) - static_cast<float>(playTextHeight)};
    exitText.pos = {
        (SDLState::logicalWidth - static_cast<float>(exitTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) + 10.0f};
  }

  void update([[maybe_unused]] float dt) {
    // Mouse coordinates come in window pixels (e.g. 1280×720) and need to be
    // converted to logical coordinates (320×180) for accurate hit-testing.
    // SDL_RenderCoordinatesFromWindow handles this transformation.
    float windowMouseX, windowMouseY;
    SDL_GetMouseState(&windowMouseX, &windowMouseY);
    float mouseX = windowMouseX;
    float mouseY = windowMouseY;
    SDL_RenderCoordinatesFromWindow(sdlState.renderer, windowMouseX,
                                    windowMouseY, &mouseX, &mouseY);

    int playTextWidth, playTextHeight;
    playText.getSize(&playTextWidth, &playTextHeight);
    if (mouseX >= playText.pos.x &&
        mouseX <= playText.pos.x + static_cast<float>(playTextWidth) &&
        mouseY >= playText.pos.y &&
        mouseY <= playText.pos.y + static_cast<float>(playTextHeight)) {
      selectedBtn = StartSceneBtns::PLAY;
    }

    int exitTextWidth, exitTextHeight;
    exitText.getSize(&exitTextWidth, &exitTextHeight);
    if (mouseX >= exitText.pos.x &&
        mouseX <= exitText.pos.x + static_cast<float>(exitTextWidth) &&
        mouseY >= exitText.pos.y &&
        mouseY <= exitText.pos.y + static_cast<float>(exitTextHeight)) {
      selectedBtn = StartSceneBtns::EXIT;
    }

    // Highlight the selected button in yellow, dim the other to the default
    // foreground color. Provides visual feedback for which option is active.
    if (selectedBtn == StartSceneBtns::PLAY) {
      playText.setColor(Colors::hl.r, Colors::hl.g, Colors::hl.b, Colors::hl.a);
      exitText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
    } else {
      playText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
      exitText.setColor(Colors::hl.r, Colors::hl.g, Colors::hl.b, Colors::hl.a);
    }
  }

  void handleEvent(const SDL_Event &event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
      switch (event.key.scancode) {
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
          selectedBtn -= 1;
          if (selectedBtn > StartSceneBtns::EXIT)
            selectedBtn = StartSceneBtns::PLAY;
          break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          selectedBtn += 1;
          if (selectedBtn > StartSceneBtns::EXIT)
            selectedBtn = StartSceneBtns::EXIT;
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
    // Draw the background image stretched to fill the entire screen.
    SDL_RenderTexture(sdlState.renderer, resourceManager.getStartSceneBgTex(),
                      nullptr, nullptr);
    playText.draw();
    exitText.draw();
  }
};
