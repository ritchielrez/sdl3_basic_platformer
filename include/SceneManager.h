#pragma once

#include "DeathScene.h"
#include "EndScene.h"
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
  end,
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
  EndScene endScene;

  SceneManager(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sceneType(SceneType::start),
        startScene(sdlState, resourceManager),
        gameScene(sdlState, resourceManager),
        deathScene(sdlState, resourceManager),
        endScene(sdlState, resourceManager) {}

  // Tick the active scene. Also checks for scene transitions:
  //   - Start → Game when the player presses "Play"
  //   - Game → Death once the death animation finishes
  //   - Game → End when the player reaches the flagpost
  //   - Death → Game (retry) or Death → Start (back to menu)
  //   - End → Game (retry) or End → Start (back to menu)
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
        // Check level completion before death so reaching the flagpost
        // takes priority over a simultaneous death.
        if (gameScene.shouldLevelComplete) {
          endScene.setPoints(gameScene.collectedCoins,
                             gameScene.slainSlimes);
          sceneType = SceneType::end;
          break;
        }
        // Wait for the death animation to finish playing before showing the
        // death screen — gives the player visual feedback of dying.
        if (player.anims[PlayerAnim::death].isStarted() &&
            player.anims[PlayerAnim::death].isTimeOut())
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
      case SceneType::end: {
        endScene.update(dt);
        if (endScene.shouldRetry) {
          gameScene.reset();
          sceneType = SceneType::game;
          endScene.shouldRetry = false;
        } else if (endScene.shouldBeBackToStart) {
          gameScene.reset();
          sceneType = SceneType::start;
          endScene.shouldBeBackToStart = false;
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
      case SceneType::end:
        endScene.handleEvent(event);
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
      case SceneType::end:
        endScene.draw();
        break;
    }
  }
};
