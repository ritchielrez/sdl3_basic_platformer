#pragma once

#include "DeathScene.h"
#include "EndScene.h"
#include "GameScene.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StartScene.h"
#include "Timer.h"

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

enum class TransitionPhase { none, fadingOut, fadingIn };

class SceneManager {
  const SDLState &sdlState;
  SceneType current;
  TransitionPhase phase = TransitionPhase::none;
  Timer timer{0.5f};
  SceneType target;

 public:
  // All three scenes are instantiated at startup and persist for the game's
  // lifetime. The GameScene is reset (tiles, entities, player) when the player
  // retries or returns to the start menu — avoiding a full reconstruction.
  StartScene startScene;
  GameScene gameScene;
  DeathScene deathScene;
  EndScene endScene;

  SceneManager(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        current(SceneType::start),
        startScene(sdlState, resourceManager),
        gameScene(sdlState, resourceManager),
        deathScene(sdlState, resourceManager),
        endScene(sdlState, resourceManager) {}

  void startTransition(SceneType dest) {
    phase = TransitionPhase::fadingOut;
    target = dest;
    timer.reset();
  }

  void updateSceneOnly(float dt) {
    switch (current) {
      case SceneType::start:
        startScene.update(dt);
        break;
      case SceneType::game:
        gameScene.update(dt);
        break;
      case SceneType::death:
        deathScene.update(dt);
        break;
      case SceneType::end:
        endScene.update(dt);
        break;
    }
  }

  // Tick the active scene. Also checks for scene transitions:
  //   - Start → Game when the player presses "Play"
  //   - Game → Death once the death animation finishes
  //   - Game → End when the player reaches the flagpost
  //   - Death → Game (retry) or Death → Start (back to menu)
  //   - End → Game (retry) or End → Start (back to menu)
  void update(float dt) {
    if (phase != TransitionPhase::none) {
      timer.step(dt);
      if (phase == TransitionPhase::fadingOut) {
        // Phase 1: fade out — freeze the outgoing scene
        if (timer.isTimeOut()) {
          current = target;
          phase = TransitionPhase::fadingIn;
          timer.reset();
        }
      } else {
        // Phase 2: fade in — new scene runs behind shrinking overlay
        updateSceneOnly(dt);
        if (timer.isTimeOut()) {
          phase = TransitionPhase::none;
        }
      }
      return;
    }

    switch (current) {
      case SceneType::start: {
        startScene.update(dt);
        if (startScene.shouldStartGame) {
          startTransition(SceneType::game);
          startScene.shouldStartGame = false;
        }
        break;
      }
      case SceneType::game: {
        const Player &player = gameScene.player;
        gameScene.update(dt);
        if (gameScene.shouldLevelComplete) {
          endScene.setPoints(gameScene.collectedCoins, gameScene.slainSlimes);
          startTransition(SceneType::end);
          break;
        }
        if (player.anims[PlayerAnim::death].isStarted() &&
            player.anims[PlayerAnim::death].isTimeOut())
          startTransition(SceneType::death);
        break;
      }
      case SceneType::death: {
        deathScene.update(dt);
        if (deathScene.shouldRetry) {
          gameScene.reset();
          startTransition(SceneType::game);
          deathScene.shouldRetry = false;
        } else if (deathScene.shouldBeBackToStart) {
          gameScene.reset();
          startTransition(SceneType::start);
          deathScene.shouldBeBackToStart = false;
        }
        break;
      }
      case SceneType::end: {
        endScene.update(dt);
        if (endScene.shouldRetry) {
          gameScene.reset();
          startTransition(SceneType::game);
          endScene.shouldRetry = false;
        } else if (endScene.shouldBeBackToStart) {
          gameScene.reset();
          startTransition(SceneType::start);
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
    switch (current) {
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
    switch (current) {
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
    if (phase != TransitionPhase::none) {
      float t = timer.getTime() / timer.getLen();
      float alpha = phase == TransitionPhase::fadingOut ? t : 1.0f - t;
      SDL_SetRenderDrawColor(sdlState.renderer, 0, 0, 0,
                             static_cast<uint8_t>(alpha * 255.0f));
      SDL_SetRenderDrawBlendMode(sdlState.renderer, SDL_BLENDMODE_BLEND);
      SDL_RenderFillRect(sdlState.renderer, nullptr);
    }
  }
};
