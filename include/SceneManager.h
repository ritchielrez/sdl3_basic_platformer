#pragma once

#include "DeathScene.h"
#include "GameScene.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StartScene.h"

// Finite-state machine driving the top-level game flow. The game transitions
// between three scenes:
//   Start → Game → Death → (Retry → Game | Back To Start → Start)
// Each scene owns its own update/draw/handleEvent logic; the SceneManager
// delegates to whichever scene is currently active.
enum class SceneType {
  start,
  game,
  death,
};

class SceneManager {
  SceneType sceneType;

 public:
  // All three scenes are instantiated at startup and persist for the game's
  // lifetime. The GameScene is reset (tiles, entities, player) when the player
  // retries or returns to the start menu — avoiding a full reconstruction.
  StartScene startScene;
  GameScene gameScene;
  DeathScene deathScene;

  SceneManager(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sceneType(SceneType::start),
        startScene(sdlState, resourceManager),
        gameScene(sdlState, resourceManager),
        deathScene(sdlState, resourceManager) {}

  // Tick the active scene. Also checks for scene transitions:
  //   - Start → Game when the player presses "Play"
  //   - Game → Death once the death animation finishes
  //   - Death → Game (retry) or Death → Start (back to menu)
  void update(float dt) {
    switch (sceneType) {
      case SceneType::start: {
        startScene.update(dt);
        if (startScene.shouldStartGame) {
          sceneType = SceneType::game;
          startScene.shouldStartGame = false;
        }
        break;
      }
      case SceneType::game: {
        const Player &player = gameScene.player;
        gameScene.update(dt);
        // Wait for the death animation to finish playing before showing the
        // death screen — gives the player visual feedback of dying.
        if (player.death && player.anims[player.currAnim].isTimeOut())
          sceneType = SceneType::death;
        break;
      }
      case SceneType::death: {
        deathScene.update(dt);
        if (deathScene.shouldRetry) {
          gameScene.reset();
          sceneType = SceneType::game;
          deathScene.shouldRetry = false;
        } else if (deathScene.shouldBeBackToStart) {
          gameScene.reset();
          sceneType = SceneType::start;
          deathScene.shouldBeBackToStart = false;
        }
        break;
      }
    }
  }

  // Only the menu scenes (start, death) handle keyboard/mouse events. During
  // gameplay, input is polled directly from SDL_GetKeyboardState for
  // responsiveness (see Player::update).
  void handleEvent(const SDL_Event &event) {
    switch (sceneType) {
      case SceneType::start:
        startScene.handleEvent(event);
        break;
      case SceneType::death:
        deathScene.handleEvent(event);
        break;
      default:
        break;
    }
  }

  void draw() {
    switch (sceneType) {
      case SceneType::start:
        startScene.draw();
        break;
      case SceneType::game:
        gameScene.draw();
        break;
      case SceneType::death:
        deathScene.draw();
        break;
    }
  }
};
