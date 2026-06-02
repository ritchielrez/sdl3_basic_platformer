#include "Player.h"

#include <SDL3/SDL.h>

#include <cstdint>

#include "Coin.h"
#include "DynTile.h"
#include "Map.h"
#include "Slime.h"
#include "StaticTile.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include "SDLState.h"

void Player::update(const SDLState& sdlState, SDL_FRect& cam, float worldWidth,
                    const std::vector<StaticTile>& staticTiles,
                    const std::vector<DynTile>& dynTiles,
                    std::vector<Coin>& coins, size_t& collectedCoins,
                    std::vector<Slime>& slimes, size_t& slainSlimes, float dt) {
  // Save previous grounded state and update for this frame.
  // Must happen before canJump is computed so wasGrounded reflects last frame.
  wasGrounded = grounded;

  // --- Jump Buffering: Record a jump press for use on the next landing ---
  const bool jumpDown = sdlState.keys[SDL_SCANCODE_SPACE];

  // We only want the rising edge of the Space key press.
  const bool jumpJustPressed = jumpDown && !wasJumpDown;
  wasJumpDown = jumpDown;

  // Reset the jump buffer timer on a new jump key press, and step it forward
  // otherwise. This is because we don't want multiple jump key events to queue
  // up.
  if (jumpJustPressed) {
    jumpBufferTimer.reset();
    jumpBufferTimer.step(dt);
  } else if (jumpBufferTimer.isStarted() && !jumpBufferTimer.isTimeOut()) {
    jumpBufferTimer.step(dt);
  }

  // Determine whether the player is allowed to jump right now:
  //   - grounded, OR within the coyote window
  const bool canJump =
      (wasGrounded || (coyoteTimer.isStarted() && !coyoteTimer.isTimeOut())) &&
      currAnim != PlayerAnim::death;

  // Trigger jump if:
  //   - Space was just pressed (or is buffered from a recent press), AND
  //   - the player is in a jumpable state
  const bool jumpBuffered =
      jumpBufferTimer.isStarted() && !jumpBufferTimer.isTimeOut();

  if (canJump && (jumpJustPressed || jumpBuffered)) {
    vel.y = jumpVel;
    currAnim = PlayerAnim::jump;
    // Consume both the buffer and the coyote window so they don't re-trigger.
    jumpBufferTimer.reset();
    coyoteTimer.reset();
  }

  // --- Variable Jump Height: Cut upward velocity when Space is released ---
  // Only applies while the player is still rising from a jump.
  if (!jumpDown && vel.y < 0) {
    vel.y *= 0.90f;  // Dampen rise per-frame; acts as a soft cut
  }

  if (dashCooldown.isStarted() && !dashCooldown.isTimeOut()) {
    dashCooldown.step(dt);
  }

  if ((currAnim == PlayerAnim::run || currAnim == PlayerAnim::jump) &&
      (!dashCooldown.isStarted() || dashCooldown.isTimeOut()) &&
      sdlState.keys[SDL_SCANCODE_LSHIFT]) {
    dashDuration.reset();
    dashCooldown.reset();
    dashDuration.step(dt);
    dashCooldown.step(dt);
  }

  if (dashDuration.isStarted() && !dashDuration.isTimeOut()) {
    vel.x += static_cast<float>(dir) * dashSpeed * dt;
    dashDuration.step(dt);
  }

  int16_t currDir = 0;
  if (currAnim != PlayerAnim::death && sdlState.keys[SDL_SCANCODE_A]) {
    currDir -= 1;
  }
  if (currAnim != PlayerAnim::death && sdlState.keys[SDL_SCANCODE_D]) {
    currDir += 1;
  }
  if (currDir != 0) {
    dir = currDir;
  }

  switch (currAnim) {
    case PlayerAnim::idle: {
      if (currDir != 0) {
        currAnim = PlayerAnim::run;
      } else if (vel.x != 0) {
        const float deaccelFactor = vel.x > 0 ? -1.5f : 1.5f;
        float deaccelVel = deaccelFactor * accel.x * dt;

        if (glm::abs(vel.x) < glm::abs(deaccelVel)) {
          vel.x = 0;
        } else {
          vel.x += deaccelVel;
        }
      }
      break;
    }
    case PlayerAnim::run: {
      if (currDir == 0) {
        currAnim = PlayerAnim::idle;
      }

      // NOTE: If `vel.x` and `dir` have different signs, their product is
      // less than zero.
      if (vel.x * static_cast<float>(dir) < 0 && grounded) {
        currAnim = PlayerAnim::slide;
      }
      break;
    }
    case PlayerAnim::slide: {
      // NOTE: If `vel.x` and `currDir` have the same signs, their product is
      // greater than zero.
      if (vel.x * static_cast<float>(currDir) > 0 && grounded) {
        currAnim = PlayerAnim::run;
      } else if (vel.x * static_cast<float>(currDir) == 0 && grounded) {
        currAnim = PlayerAnim::idle;
      }
      break;
    }
  }

  vel += static_cast<float>(currDir) * accel * dt;

  if (!dashDuration.isStarted() || dashDuration.isTimeOut()) {
    vel.x = glm::clamp(vel.x, -maxSpeed.x, maxSpeed.x);
  } else {
    vel.x = glm::clamp(vel.x, -maxSpeed.x - dashSpeed, maxSpeed.x + dashSpeed);
  }

  constexpr float gravity = 980.0f;
  if (!grounded) vel.y += gravity * dt;

  vel.y = glm::clamp(vel.y, -maxSpeed.y, maxSpeed.y);

  glm::vec2 velFrame = vel * dt;

  if (velFrame.y >= Map::TILE_SIZE) {
    velFrame.y = Map::TILE_SIZE - 1;
    vel.y = velFrame.y / dt;
  }

  pos += velFrame;
  if (pos.x <= 0)
    pos.x = 0;
  else if (pos.x + w >= worldWidth)
    pos.x = worldWidth - w;
  collision(staticTiles, dynTiles, coins, collectedCoins, slimes, slainSlimes,
            dt);

  if (currAnim != PlayerAnim::death) {
    // --- Horizontal Camera System ---
    // The 'camRuler' is the point where the player is exactly in the center of
    // the screen.
    float camRuler = (SDLState::logicalWidth - w) / 2;
    constexpr float camXSmoothness = 3.0f;
    float targetX = pos.x - camRuler;

    // Velocity Look-Ahead:
    // Standard lerp smoothing causes a steady-state lag where the camera trails
    // the player. We offset the target by a fraction of the velocity to
    // compensate.
    targetX += vel.x * 0.20f;

    // Threshold Logic: The camera stays at 0 until the player first reaches the
    // center of the screen, after which it begins smooth tracking.
    if (!passedCamRuler && pos.x >= camRuler) passedCamRuler = true;
    if (passedCamRuler) {
      if (pos.x + SDLState::logicalWidth >= worldWidth) {
        cam.x = glm::lerp(cam.x, worldWidth - SDLState::logicalWidth,
                          camXSmoothness / 2 * dt);
      } else if (pos.x >= camRuler &&
                 !(dashDuration.isStarted() && !dashDuration.isTimeOut())) {
        cam.x = glm::lerp(cam.x, targetX, camXSmoothness * dt);
      } else if (pos.x >= camRuler) {
        cam.x = glm::lerp(cam.x, targetX, camXSmoothness / 2 * dt);
      } else {
        cam.x = glm::lerp(cam.x, 0.0f, camXSmoothness * dt);
      }
    }

    // --- Vertical Camera System ---
    // If the player is close to the top of the screen, then move the camera up
    // slightly so it does not look like the player is touching the ceiling or
    // going beyond it.
    constexpr float camYSmoothness = 5.0f;

    if (pos.y <= 10) {
      cam.y = glm::lerp(cam.y, -30.0f, camYSmoothness * dt);
    } else {
      cam.y = glm::lerp(cam.y, 0.0f, camYSmoothness * dt);
    }

    // --- Coyote Time: Allow jumping briefly after walking off a ledge ---
    // Start the coyote window on the first frame the player becomes airborne
    // without having jumped (i.e., walked off an edge).
    if (wasGrounded && !grounded && currAnim != PlayerAnim::jump) {
      coyoteTimer.reset();
      coyoteTimer.step(dt);
    } else if (coyoteTimer.isStarted() && !coyoteTimer.isTimeOut()) {
      coyoteTimer.step(dt);
    }

    if (currAnim == PlayerAnim::jump && grounded) {
      currAnim = PlayerAnim::run;
    }
  }
}

void Player::collision(const std::vector<StaticTile>& staticTiles,
                       const std::vector<DynTile>& dynTiles,
                       std::vector<Coin>& coins, size_t& collectedCoins,
                       std::vector<Slime>& slimes, size_t& slainSlimes,
                       float dt) {
  SDL_FRect playerCollider{.x = pos.x + collider.x,
                           .y = pos.y + collider.y,
                           .w = collider.w,
                           .h = collider.h};
  SDL_FRect collidedRect{0, 0, 0, 0};
  SDL_FRect intersectionRect{0, 0, 0, 0};

  collided = false;
  bool foundGround = false;
  bool collidedWithDynTile = false;

  for (auto& staticTile : staticTiles) {
    collidedRect.x = staticTile.pos.x + staticTile.collider.x;
    collidedRect.y = staticTile.pos.y + staticTile.collider.y;
    collidedRect.w = staticTile.collider.w;
    collidedRect.h = staticTile.collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;

      if (intersectionRect.w > intersectionRect.h) {
        if (vel.y > 0) {
          pos.y -= intersectionRect.h;
        } else if (vel.y < 0) {
          pos.y += intersectionRect.h;
        }
        vel.y = 0;
      } else {
        if (vel.x > 0) {
          pos.x -= intersectionRect.w;
        } else if (vel.x < 0) {
          pos.x += intersectionRect.w;
        }
        vel.x = 0;
      }

      // Recalculate playerCollider after the player has moved due to collision.
      playerCollider.x = pos.x + collider.x;
      playerCollider.y = pos.y + collider.y;

      if (playerCollider.y + playerCollider.h <= collidedRect.y) {
        foundGround = true;
      }
    }
  }

  for (auto& dynTile : dynTiles) {
    collidedRect.x = dynTile.pos.x + dynTile.collider.x;
    collidedRect.y = dynTile.pos.y + dynTile.collider.y;
    collidedRect.w = dynTile.collider.w;
    collidedRect.h = dynTile.collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;
      collidedWithDynTile = true;

      if (intersectionRect.w > intersectionRect.h) {
        if (vel.y > 0) {
          pos.y -= intersectionRect.h;
        } else if (vel.y < 0) {
          pos.y += intersectionRect.h;
        }
        vel.y = 0;
      } else {
        if (vel.x > 0) {
          pos.x -= intersectionRect.w;
        } else if (vel.x < 0) {
          pos.x += intersectionRect.w;
        }
        vel.x = 0;
      }

      // Recalculate playerCollider after the player has moved due to collision.
      playerCollider.x = pos.x + collider.x;
      playerCollider.y = pos.y + collider.y;

      if (playerCollider.y + playerCollider.h <= collidedRect.y) {
        foundGround = true;
      }

      // The `dynTile` velocity carries or pushes the player.
      pos.x += dynTile.vel.x * dt;
      // A player can collide with one `dynTile` at a time, so if detected
      // collision with one, stop checking for more collisions with other
      // `dynTiles`.
      break;
    }
  }

  // If the player collides with a `dynTile` then immediately collides with a
  // `staticTile`, it means the player is being squished between two tiles. In
  // that case the player should die.
  if (collidedWithDynTile) {
    for (auto& staticTile : staticTiles) {
      collidedRect.x = staticTile.pos.x + staticTile.collider.x;
      collidedRect.y = staticTile.pos.y + staticTile.collider.y;
      collidedRect.w = staticTile.collider.w;
      collidedRect.h = staticTile.collider.h;

      if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                       &intersectionRect)) {
        collided = true;

        if (intersectionRect.w > intersectionRect.h) {
          if (vel.y > 0) {
            pos.y -= intersectionRect.h;
          } else if (vel.y < 0) {
            pos.y += intersectionRect.h;
          }
          vel.y = 0;
        } else {
          if (vel.x > 0) {
            pos.x -= intersectionRect.w;
          } else if (vel.x < 0) {
            pos.x += intersectionRect.w;
          }
          vel.x = 0;
        }

        // Recalculate playerCollider after the player has moved due to
        // collision.
        playerCollider.x = pos.x + collider.x;
        playerCollider.y = pos.y + collider.y;

        if (playerCollider.y + playerCollider.h <= collidedRect.y) {
          foundGround = true;
        } else {
          currAnim = PlayerAnim::death;
        }
      }
    }
  }

  for (size_t j = 0; j < coins.size();) {
    collidedRect.x = coins[j].pos.x + coins[j].collider.x;
    collidedRect.y = coins[j].pos.y + coins[j].collider.y;
    collidedRect.w = coins[j].collider.w;
    collidedRect.h = coins[j].collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;
      coins[j] = coins.back();
      coins.pop_back();
      collectedCoins++;
    } else {
      j++;
    }
  }

  for (size_t k = 0; k < slimes.size();) {
    collidedRect.x = slimes[k].pos.x + slimes[k].collider.x;
    collidedRect.y = slimes[k].pos.y + slimes[k].collider.y;
    collidedRect.w = slimes[k].collider.w;
    collidedRect.h = slimes[k].collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;

      if (vel.y > 0 &&
          (playerCollider.y + playerCollider.h - intersectionRect.h) <=
              slimes[k].pos.y + slimes[k].collider.y + 5.0f) {
        slimes[k] = slimes.back();
        slimes.pop_back();
        slainSlimes++;
        continue;
      }
      currAnim = PlayerAnim::death;
    }
    k++;
  }

  if (currAnim != PlayerAnim::death && grounded != foundGround) {
    grounded = foundGround;
    if (foundGround) {
      currAnim = PlayerAnim::run;
    }
  }
}
