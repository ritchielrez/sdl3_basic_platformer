#pragma once

#include <fmt/core.h>

#include <string>

#include "Coin.h"
#include "DynTile.h"
#include "Entity.h"
#include "SDLState.h"
#include "Slime.h"
#include "StaticTile.h"

// Animation state IDs for the player. Each maps to an index in the `anims`
// vector inherited from Entity. The game switches between these based on
// movement state (e.g. idle → run when moving, run → jump in air).
namespace PlayerAnim {
enum { idle, run, jump, slide, death };
}

// The player character. Extends Entity with platformer-specific movement:
// acceleration-based physics, jumping, dashing, and input buffering for
// responsive feel. This is the most gameplay-critical struct in the game.
struct Player : public Entity {
  // Acceleration (pixels/s²) and maximum speed (pixels/s). Movement uses
  // acceleration rather than instant velocity — this gives a smooth, analog
  // feel instead of digital on/off. The player accelerates toward maxSpeed
  // while a direction key is held and decelerates (friction) when released.
  glm::vec2 accel, maxSpeed;
  // Initial upward velocity applied when the player jumps (pixels/s).
  float jumpVel;
  // Instant horizontal velocity applied during a dash (pixels/s).
  float dashSpeed;
  // True when the player is intersecting a solid tile this frame.
  bool collided;
  // True when the player is standing on solid ground (vel.y == 0 after
  // collision resolution).
  bool grounded;
  // Grounded state from the previous frame. Used to detect transitions
  // (landing: wasGrounded=false → grounded=true; falling: reverse).
  bool wasGrounded;
  // True during the frame the jump key was pressed. Used to prevent
  // re-triggering jumps while the key is held.
  bool wasJumpDown;
  // Camera follows the player but stops scrolling past this invisible
  // "ruler" line so the player sees what's ahead. False → camera tracks
  // player; True → camera is locked until the player moves past the ruler.
  bool passedCamRuler;

  // --- Movement mechanic timers ---

  // How long the dash lasts once activated (0.75s). While running, the player
  // moves at dashSpeed in the held direction.
  Timer dashDuration;
  // Cooldown before the player can dash again (5.0s). Prevents spam-dashing.
  Timer dashCooldown;

  // --- Movement polish timers ("game feel") ---

  // Coyote time: a brief window (0.1s = 100ms) after the player walks off a
  // ledge during which they can still jump. This compensates for imperfect
  // timing and makes edge-jumps feel fair rather than punishing.
  Timer coyoteTimer;
  // Jump buffer: if the player presses jump slightly before landing (up to
  // 0.12s = 120ms), the input is remembered and the jump executes on the
  // first grounded frame. This eliminates the "I pressed jump but nothing
  // happened" frustration.
  Timer jumpBufferTimer;

  Player()
      : accel(glm::vec2(0)),
        maxSpeed(glm::vec2(0)),
        jumpVel(0),
        dashSpeed(0),
        collided(false),
        grounded(false),
        wasGrounded(false),
        wasJumpDown(false),
        passedCamRuler(false),
        dashDuration(0.75f),
        dashCooldown(5.0f),
        coyoteTimer(0.1f),
        jumpBufferTimer(0.12f) {}

  // Main update: reads input, applies movement physics, runs collision
  // detection/resolution, and steps animations. Also updates the camera to
  // follow the player.
  void update(const SDLState& sdlState, SDL_FRect& cam,
              const std::vector<StaticTile>& staticTiles,
              const std::vector<DynTile>& dynTiles, std::vector<Coin>& coins,
              size_t& collectedCoins, std::vector<Slime>& slimes,
              size_t& slainSlimes, float dt);

  // AABB-vs-AABB collision detection and response. Iterates all solid tiles
  // and moving platforms, pushes the player out of overlaps, and sets
  // grounded/collided flags. Also checks coin pickups and slime stomps.
  void collision(const std::vector<StaticTile>& staticTiles,
                 const std::vector<DynTile>& dynTiles, std::vector<Coin>& coins,
                 size_t& collectedCoins, std::vector<Slime>& slimes,
                 size_t& slainSlimes, float dt);

  // Returns a human-readable string of the player's current state for the
  // debug overlay. Exposes position, velocity, animation state, and all
  // timer statuses (dash, cooldown, coyote, jump buffer).
  [[nodiscard]] std::string inspect() const {
    std::string playerState{8, 0};
    switch (currAnim) {
      case PlayerAnim::idle: {
        playerState = "idle";
        break;
      }
      case PlayerAnim::run: {
        playerState = "run";
        break;
      }
      case PlayerAnim::slide: {
        playerState = "slide";
        break;
      }
      case PlayerAnim::jump: {
        playerState = "jump";
        break;
      }
      case PlayerAnim::death: {
        playerState = "death";
        break;
      }
    }
    return fmt::format(
        "Position: ({}, {})\nVelocity: ({}, {})\nState: {}\nCollision: "
        "{}\nGrounded: {}\nWas grounded: {}\nPressed jump down (last frame): "
        "{}\nDash duration active: {}\nDash cooldown active: "
        "{}\nCoyote active: {}\nJump buffered: {}\n",
        pos.x, pos.y, vel.x, vel.y, playerState, collided, grounded,
        wasGrounded, wasJumpDown,
        dashDuration.isStarted() && !dashDuration.isTimeOut(),
        dashCooldown.isStarted() && !dashCooldown.isTimeOut(),
        coyoteTimer.isStarted() && !coyoteTimer.isTimeOut(),
        jumpBufferTimer.isStarted() && !jumpBufferTimer.isTimeOut());
  }
};
