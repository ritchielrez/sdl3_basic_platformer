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

// The title/main menu scene. Displays a background image, a "Play" button,
// and an "Exit" button. Supports both keyboard navigation (WASD / arrows +
// Enter/Space) and mouse (hover + click).
class StartScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  Text playText;  // "Play" button --- starts the game
  Text exitText;  // "Exit" button --- closes the game
  // Currently highlighted button index (0 = Play, 1 = Exit).
  uint8_t selectedBtn;

 public:
  // Set to true when the player confirms "Play". Read by SceneManager to
  // transition to the game scene.
  bool shouldStartGame = false;
  // Set to true when the player confirms "Exit" or clicks the window close
  // button. Read by Game::run to break the main loop.
  bool shouldQuit = false;

  // Construct the start menu. Creates "Play" and "Exit" text objects,
  // positions them centered on screen, and defaults to the Play button
  // being selected.
  StartScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        playText(sdlState, "Play", glm::vec2(0)),
        exitText(sdlState, "Exit", glm::vec2(0)),
        selectedBtn(StartSceneBtns::PLAY) {
    int playTextWidth, playTextHeight, exitTextWidth, exitTextHeight;
    playText.getSize(&playTextWidth, &playTextHeight);
    exitText.getSize(&exitTextWidth, &exitTextHeight);

    // Position buttons centered horizontally, stacked vertically around the
    // middle of the logical screen (320×180).
    playText.pos = {
        (SDLState::logicalWidth - static_cast<float>(playTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) -
            static_cast<float>(playTextHeight) / 2.0f};
    exitText.pos = {
        (SDLState::logicalWidth - static_cast<float>(exitTextWidth)) / 2.0f,
        (SDLState::logicalHeight / 2.0f) + 10.0f};
  }

  // Poll mouse position for hover highlighting of buttons. Converts
  // window-space coordinates to logical (320x180) for accurate hit-testing.
  // Updates button colors based on which button is selected.
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

    // Calculate widths and heights of all texts. These calculations help us 
    // detect what button the user is hovering on. Change `selectedBtn` based
    // on the button the user is hovering on.
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

    // Highlight the selected button in highlighting foreground color set in Colors.h, 
    // dim the other to the default foreground color. Provides visual feedback for which 
    // option is active.
    if (selectedBtn == StartSceneBtns::PLAY) {
      playText.setColor(Colors::hl.r, Colors::hl.g, Colors::hl.b, Colors::hl.a);
      exitText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
    } else {
      playText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
      exitText.setColor(Colors::hl.r, Colors::hl.g, Colors::hl.b, Colors::hl.a);
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
          if (selectedBtn > StartSceneBtns::EXIT)
            selectedBtn = StartSceneBtns::PLAY;
          break;
        // DOWN / S — highlight the next button.
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          selectedBtn += 1;
          // If we moved past the last button, stay on the last one.
          if (selectedBtn > StartSceneBtns::EXIT)
            selectedBtn = StartSceneBtns::EXIT;
          break;
        // ENTER / SPACE — activate whichever button is currently highlighted.
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

        int playTextWidth, playTextHeight, exitTextWidth, exitTextHeight;
        playText.getSize(&playTextWidth, &playTextHeight);
        exitText.getSize(&exitTextWidth, &exitTextHeight);

        // Now check if the converted click falls inside any button's area.
        // A button is just a rectangle — we compare the mouse X and Y
        // against its left, right, top, and bottom edges.
        if (mouseX >= playText.pos.x &&
            mouseX <= playText.pos.x + static_cast<float>(playTextWidth) &&
            mouseY >= playText.pos.y &&
            mouseY <= playText.pos.y + static_cast<float>(playTextHeight)) {
          // Clicked "Play" — start the game.
          shouldStartGame = true;
        } else if (mouseX >= exitText.pos.x &&
                   mouseX <=
                       exitText.pos.x + static_cast<float>(exitTextWidth) &&
                   mouseY >= exitText.pos.y &&
                   mouseY <=
                       exitText.pos.y + static_cast<float>(exitTextHeight)) {
          // Clicked "Exit" — close the game.
          shouldQuit = true;
        }
      }
    }
  }

  // Draw the background image and both menu buttons (Play, Exit) at
  // their pre-computed screen positions.
  void draw() {
    // Draw the background image stretched to fill the entire screen.
    SDL_RenderTexture(sdlState.renderer, resourceManager.getStartSceneBgTex(),
                      nullptr, nullptr);
    playText.draw();
    exitText.draw();
  }
};
