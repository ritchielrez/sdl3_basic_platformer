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
//
// Scene changes use a two-phase cross-fade transition:
//   Phase 1 (fadingOut): a black overlay fades in over the current scene,
//                         hiding it completely. The scene is NOT updated so
//                         it appears frozen.
//   Phase 2 (fadingIn):  the new scene is swapped in and begins updating
//                         behind the black overlay, which now fades out to
//                         reveal it.
// Between phases the current scene is atomically swapped to the target,
// invisible behind the fully opaque overlay.
enum class SceneType {
  start,
  game,
  death,
  end,
};

enum class TransitionPhase { none, fadingOut, fadingIn };

class SceneManager {
  const SDLState &sdlState;
  SceneType current;           // The scene currently being rendered (and updated when not transitioning)
  SceneType target;            // The scene to switch to once the fade-out completes
  TransitionPhase phase = TransitionPhase::none;  // Which stage of the cross-fade we are in
  Timer timer{0.75f};          // Duration of each transition half (fade-out and fade-in), in seconds

 public:
  // All three scenes are instantiated at startup and persist for the game's
  // lifetime. The GameScene is reset (tiles, entities, player) when the player
  // retries or returns to the start menu — avoiding a full reconstruction.
  StartScene startScene;
  GameScene gameScene;
  DeathScene deathScene;
  EndScene endScene;

  // Construct the scene state machine. All four scenes (start, game, death,
  // end) are created immediately and persist for the game's lifetime.
  SceneManager(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        current(SceneType::start),
        target(SceneType::start),
        startScene(sdlState, resourceManager),
        gameScene(sdlState, resourceManager),
        deathScene(sdlState, resourceManager),
        endScene(sdlState, resourceManager) {}

  // Begin a cross-fade transition to the destination scene. The current
  // scene freezes while fading to black, then the new scene appears.
  //
  // This kicks off Phase 1 (fadingOut): the current scene stops updating and
  // a black overlay grows opaque over it. Once fully black, the scene pointer
  // is swapped to `dest` behind the overlay (in update()), and Phase 2
  // (fadingIn) begins — the new scene updates while the overlay fades out.
  void startTransition(SceneType dest) {
    phase = TransitionPhase::fadingOut;
    target = dest;
    timer.reset();
  }

  // Tick the currently active scene without processing any transition
  // logic. Used internally during the fade-in phase.
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
    // --- Transition handling ---
    // When a transition is active, we bypass the normal per-scene update
    // logic below and instead run the two-phase fade sequence.
    if (phase != TransitionPhase::none) {
      timer.step(dt);
      if (phase == TransitionPhase::fadingOut) {
        // Phase 1: fade out — freeze the outgoing scene.
        // The current scene is NOT updated so it appears frozen in place.
        // The draw() method paints an increasingly opaque black overlay on
        // top, so the scene gradually disappears into blackness.
        // Once the timer finishes, we atomically swap to the target scene
        // (invisible behind the fully opaque overlay) and begin fading in.
        if (timer.isTimeOut()) {
          current = target;
          phase = TransitionPhase::fadingIn;
          timer.reset();
        }
      } else {
        // Phase 2: fade in — new scene runs behind shrinking overlay.
        // The new scene (now `current`) begins updating normally, but the
        // draw() method still paints a black overlay that starts fully opaque
        // and shrinks to transparent. When the timer finishes, the transition
        // is complete and we resume normal single-scene rendering.
        updateSceneOnly(dt);
        if (timer.isTimeOut()) {
          phase = TransitionPhase::none;
        }
      }
      return;
    }

    // No transition running — tick the active scene normally and check
    // for conditions that should trigger a transition to another scene.
    switch (current) {
      case SceneType::start: {
        // Update the title menu. If the player confirmed "Play"
        // (shouldStartGame set by handleEvent), fade to the game scene.
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
        // Transition to the end/victory screen when the player reaches the flagpost.
        if (gameScene.shouldLevelComplete) {
          endScene.setPoints(gameScene.collectedCoins, gameScene.slainSlimes);
          startTransition(SceneType::end);
          break;
        }
        // Transition to the death screen once the player's death animation finishes.
        if (player.anims[PlayerAnim::death].isStarted() &&
            player.anims[PlayerAnim::death].isTimeOut())
          startTransition(SceneType::death);
        break;
      }
      case SceneType::death: {
        // Update the death menu. Player can choose retry or return to start.
        // Either path resets gameScene so the level starts fresh.
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
        // Update the victory screen. Same retry / back-to-start choices as death.
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

  // This runs every time the player does something — pressing a key, clicking
  // the mouse, etc. It checks which screen is currently active (the title menu,
  // the death screen, or the victory screen) and sends the event to that
  // screen's own handleEvent() so it can react. The gameplay scene is skipped
  // here because during gameplay we read the keyboard directly every frame
  // instead (see Player::update).
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

  // Render the currently active scene and, if a transition is in progress,
  // draw a black overlay whose alpha drives the fade-in/fade-out effect.
  //
  // The overlay is a full-screen black rectangle drawn with blending enabled.
  // Its alpha interpolates from 0 → 1 during fadingOut (the outgoing scene
  // grows darker), and from 1 → 0 during fadingIn (the new scene emerges).
  // This creates a smooth cross-fade between scenes without needing to render
  // both scenes simultaneously.
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
      // `t` goes from 0 → 1 over the duration of each transition half.
      float t = timer.getTime() / timer.getLen();
      // In fadingOut:  alpha ramps  0 → 1  (scene disappears into black).
      // In fadingIn:   alpha ramps  1 → 0  (black overlay lifts to reveal).
      float alpha = phase == TransitionPhase::fadingOut ? t : 1.0f - t;
      SDL_SetRenderDrawColor(sdlState.renderer, 0, 0, 0,
                             static_cast<uint8_t>(alpha * 255.0f));
      SDL_SetRenderDrawBlendMode(sdlState.renderer, SDL_BLENDMODE_BLEND);
      // Passing nullptr as the rect means "the entire render target".
      SDL_RenderFillRect(sdlState.renderer, nullptr);
    }
  }
};
