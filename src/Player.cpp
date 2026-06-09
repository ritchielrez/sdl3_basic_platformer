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

// Main player update: reads input, applies movement physics (acceleration,
// jumping, dashing), handles coyote time and jump buffering, then runs
// collision resolution and updates the camera to follow the player.
//
// The update order is carefully sequenced:
//   1. Save previous frame state (wasGrounded, wasJumpDown)
//   2. Process jump input with buffering and coyote time
//   3. Apply variable jump height on key release
//   4. Manage dash state (cooldown, duration, velocity impulse)
//   5. Read horizontal movement input (A/D)
//   6. Update animation state machine (idle/run/slide)
//   7. Apply acceleration, gravity, and speed limits
//   8. Run collision detection and response
//   9. Update camera to follow player
void Player::update(const SDLState& sdlState, SDL_FRect& cam, float worldWidth,
                    const std::vector<StaticTile>& staticTiles,
                    const std::vector<DynTile>& dynTiles,
                    std::vector<Coin>& coins, size_t& collectedCoins,
                    std::vector<Slime>& slimes, size_t& slainSlimes, float dt) {
  // Save previous grounded state and update for this frame.
  // Must happen before canJump is computed so wasGrounded reflects last frame.
  wasGrounded = grounded;

  // --- Jump Buffering: Record a jump press for use on the next landing ---
  // This allows the player to press Space slightly before landing and still
  // have the jump register, making the controls feel more responsive.
  const bool jumpDown = sdlState.keys[SDL_SCANCODE_SPACE];

  // We only want the rising edge of the Space key press (ignore holding).
  const bool jumpJustPressed = jumpDown && !wasJumpDown;
  wasJumpDown = jumpDown;

  // On a fresh key press, start the buffer timer. On subsequent frames while
  // the buffer is active, just advance it. Once the buffer expires, the
  // buffered jump is lost.
  if (jumpJustPressed) {
    jumpBufferTimer.reset();
    jumpBufferTimer.step(dt);
  } else if (jumpBufferTimer.isStarted() && !jumpBufferTimer.isTimeOut()) {
    jumpBufferTimer.step(dt);
  }

  // Determine whether the player is allowed to jump right now:
  //   - grounded, OR within the coyote window (brief grace period after
  //     walking off a ledge)
  const bool canJump =
      (wasGrounded || (coyoteTimer.isStarted() && !coyoteTimer.isTimeOut())) &&
      currAnim != PlayerAnim::death;

  // Trigger jump if:
  //   - Space was just pressed (or is buffered from a recent press), AND
  //   - the player is in a jumpable state (grounded or coyote)
  const bool jumpBuffered =
      jumpBufferTimer.isStarted() && !jumpBufferTimer.isTimeOut();

  if (canJump && (jumpJustPressed || jumpBuffered)) {
    vel.y = jumpVel;
    currAnim = PlayerAnim::jump;
    // Consume both the buffer and the coyote window so they don't re-trigger
    // multiple jumps from a single press or edge fall.
    jumpBufferTimer.reset();
    coyoteTimer.reset();
  }

  // --- Variable Jump Height: Cut upward velocity when Space is released ---
  // Holding Space makes the player jump full height; releasing early shortens
  // the jump. This is done by damping upward velocity each frame the key is
  // released, which gives a natural-feeling "soft cut" instead of an instant
  // stop.
  if (!jumpDown && vel.y < 0) {
    vel.y *= 0.90f;
  }

  // --- Dash state management ---
  // If a dash cooldown is active, tick it down so the player can dash again
  // once it expires.
  if (dashCooldown.isStarted() && !dashCooldown.isTimeOut()) {
    dashCooldown.step(dt);
  }

  // Trigger dash: only when running or jumping, cooldown is finished, and
  // Shift is pressed. Resetting both dashDuration and dashCooldown starts
  // the dash window and the cooldown timer simultaneously.
  if ((currAnim == PlayerAnim::run || currAnim == PlayerAnim::jump) &&
      (!dashCooldown.isStarted() || dashCooldown.isTimeOut()) &&
      sdlState.keys[SDL_SCANCODE_LSHIFT]) {
    dashDuration.reset();
    dashCooldown.reset();
    dashDuration.step(dt);
    dashCooldown.step(dt);
  }

  // Apply dash velocity impulse while the dash window is active. The impulse
  // is added on top of normal movement velocity.
  if (dashDuration.isStarted() && !dashDuration.isTimeOut()) {
    vel.x += static_cast<float>(dir) * dashSpeed * dt;
    dashDuration.step(dt);
  }

  // --- Horizontal input (A/D keys) ---
  // currDir is -1 (left), +1 (right), or 0 (none). Death animation locks
  // horizontal input so the player corpse doesn't slide around.
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

  // --- Animation state machine (idle / run / slide) ---
  // Transitions depend on horizontal input (currDir), current velocity, and
  // grounded state. The death and jump animations are set externally (by
  // collision or jump trigger) and are not managed here.
  switch (currAnim) {
    case PlayerAnim::idle: {
      // Start running if the player presses left or right.
      if (currDir != 0) {
        currAnim = PlayerAnim::run;
      } else if (vel.x != 0) {
        // No input but still moving — apply friction (deceleration at 1.5x
        // the normal acceleration rate) until velocity reaches zero.
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
      // Release horizontal input → return to idle.
      if (currDir == 0) {
        currAnim = PlayerAnim::idle;
      }

      // If the player is grounded and holding the opposite direction, skid
      // into slide animation. Detected when vel.x and dir have opposite signs.
      if (vel.x * static_cast<float>(dir) < 0 && grounded) {
        currAnim = PlayerAnim::slide;
      }
      break;
    }
    case PlayerAnim::slide: {
      // If the player starts moving in the direction they're facing again,
      // go back to running. If they release all input, return to idle.
      if (vel.x * static_cast<float>(currDir) > 0 && grounded) {
        currAnim = PlayerAnim::run;
      } else if (vel.x * static_cast<float>(currDir) == 0 && grounded) {
        currAnim = PlayerAnim::idle;
      }
      break;
    }
  }

  // --- Apply horizontal acceleration ---
  // currDir (-1, 0, +1) multiplied by accel gives the frame's velocity delta.
  // This produces smooth acceleration rather than instant max speed.
  vel += static_cast<float>(currDir) * accel * dt;

  // Clamp horizontal speed to maxSpeed (or maxSpeed + dashSpeed during a dash).
  if (!dashDuration.isStarted() || dashDuration.isTimeOut()) {
    vel.x = glm::clamp(vel.x, -maxSpeed.x, maxSpeed.x);
  } else {
    vel.x = glm::clamp(vel.x, -maxSpeed.x - dashSpeed, maxSpeed.x + dashSpeed);
  }

  // --- Apply gravity ---
  // Gravity only pulls when airborne (not grounded).
  constexpr float gravity = 980.0f;
  if (!grounded) vel.y += gravity * dt;

  vel.y = glm::clamp(vel.y, -maxSpeed.y, maxSpeed.y);

  // --- Compute frame displacement ---
  glm::vec2 velFrame = vel * dt;

  // Prevent the player from moving more than one tile per frame vertically.
  // This avoids tunnelling through thin platforms at high speeds.
  if (velFrame.y >= Map::TILE_SIZE) {
    velFrame.y = Map::TILE_SIZE - 1;
    vel.y = velFrame.y / dt;
  }

  // --- Apply displacement and clamp to world bounds ---
  pos += velFrame;
  if (pos.x <= 0)
    pos.x = 0;
  else if (pos.x + w >= worldWidth)
    pos.x = worldWidth - w;
  collision(staticTiles, dynTiles, coins, collectedCoins, slimes, slainSlimes,
            dt);

  // --- Camera and post-movement updates (only while alive) ---
  // Skip camera tracking and coyote time when the death animation is active.
  if (currAnim != PlayerAnim::death) {
    // --- Horizontal Camera System ---
    // The camera smoothly follows the player using lerp. Several refinements
    // make the tracking feel natural:
    //
    //   camRuler:     The X position that would centre the player on screen.
    //   velocity look-ahead: Offsets the target slightly in the direction of
    //                        movement to compensate for lerp lag.
    //   threshold:    The camera stays fixed at x=0 until the player walks past
    //                 the centre for the first time.
    //   edge clamp:   Smoothly snaps to the far-right when near the world edge.
    //   dash damping: Slows the camera during dashes (half smoothness) so the
    //                 sudden speed burst doesn't jerk the view.
    float camRuler = (SDLState::logicalWidth - w) / 2;
    constexpr float camXSmoothness = 3.0f;
    float targetX = pos.x - camRuler;

    // Compensate for lerp lag by looking ahead in the direction of movement.
    targetX += vel.x * 0.20f;

    // The camera stays at 0 until the player first reaches the centre of the
    // screen, then begins smooth tracking.
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
    // When the player is near the top of the screen (pos.y <= 10), shift the
    // camera upward by 30 pixels so the player doesn't appear to clip the
    // ceiling. Otherwise smoothly reset to the default vertical position.
    constexpr float camYSmoothness = 5.0f;

    if (pos.y <= 10) {
      cam.y = glm::lerp(cam.y, -30.0f, camYSmoothness * dt);
    } else {
      cam.y = glm::lerp(cam.y, 0.0f, camYSmoothness * dt);
    }

    // --- Coyote Time: Allow jumping briefly after walking off a ledge ---
    // When the player walks off a platform without pressing jump (wasGrounded
    // true → grounded false), start the coyote timer. While the timer is
    // active, the player can still jump as though they were on the ground.
    // This small grace period makes edge-of-platform gameplay feel less
    // punishing.
    if (wasGrounded && !grounded && currAnim != PlayerAnim::jump) {
      coyoteTimer.reset();
      coyoteTimer.step(dt);
    } else if (coyoteTimer.isStarted() && !coyoteTimer.isTimeOut()) {
      coyoteTimer.step(dt);
    }

    // Transition out of the jump animation back to run once the player lands.
    if (currAnim == PlayerAnim::jump && grounded) {
      currAnim = PlayerAnim::run;
    }
  }
}

// AABB collision detection and response against static tiles, moving
// platforms, coins, and slimes. Pushes the player out of overlaps, handles
// coin pickups, slime stomps, and squish death from moving platforms.
//
// The collision resolution uses axis-separated push-out: for each overlapping
// tile, we compare the intersection rect's width vs height to determine which
// axis had the shallowest penetration, then push the player out along that
// axis and zero the corresponding velocity. This prevents the player from
// sliding through walls or getting stuck.
void Player::collision(const std::vector<StaticTile>& staticTiles,
                       const std::vector<DynTile>& dynTiles,
                       std::vector<Coin>& coins, size_t& collectedCoins,
                       std::vector<Slime>& slimes, size_t& slainSlimes,
                       float dt) {
  // Compute the player's effective collision box (sprite position + collider
  // offset). This is typically smaller than the sprite for fairer hitboxes.
  SDL_FRect playerCollider{.x = pos.x + collider.x,
                           .y = pos.y + collider.y,
                           .w = collider.w,
                           .h = collider.h};
  SDL_FRect collidedRect{0, 0, 0, 0};
  SDL_FRect intersectionRect{0, 0, 0, 0};

  collided = false;
  bool foundGround = false;
  bool collidedWithDynTile = false;

  // --- Static tile collision ---
  // Iterate all ground/terrain tiles. On overlap, push the player out of the
  // tile along the shallowest axis and zero the corresponding velocity.
  for (auto& staticTile : staticTiles) {
    collidedRect.x = staticTile.pos.x + staticTile.collider.x;
    collidedRect.y = staticTile.pos.y + staticTile.collider.y;
    collidedRect.w = staticTile.collider.w;
    collidedRect.h = staticTile.collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;

      // Resolve the collision along the shallowest penetration axis.
      // If the overlap is wider than tall, it's a vertical collision (hit
      // floor or ceiling). Otherwise it's horizontal (hit a wall).
      if (intersectionRect.w > intersectionRect.h) {
        // Vertical push: player was moving down (landing) or up (bonking head).
        if (vel.y > 0) {
          pos.y -= intersectionRect.h;
        } else if (vel.y < 0) {
          pos.y += intersectionRect.h;
        }
        vel.y = 0;
      } else {
        // Horizontal push: player was moving right or left into a wall.
        if (vel.x > 0) {
          pos.x -= intersectionRect.w;
        } else if (vel.x < 0) {
          pos.x += intersectionRect.w;
        }
        vel.x = 0;
      }

      // Recalculate player collider after position correction so subsequent
      // tile checks use the updated bounds.
      playerCollider.x = pos.x + collider.x;
      playerCollider.y = pos.y + collider.y;

      // If the bottom of the player collider is at or above the top of the
      // tile, the player is standing on this tile (grounded).
      if (playerCollider.y + playerCollider.h <= collidedRect.y) {
        foundGround = true;
      }
    }
  }

  // --- Moving platform collision ---
  // Same AABB resolution as static tiles, but also applies the platform's
  // velocity to carry the player along. Only one dynTile collision per frame
  // is processed (break after first hit).
  for (auto& dynTile : dynTiles) {
    collidedRect.x = dynTile.pos.x + dynTile.collider.x;
    collidedRect.y = dynTile.pos.y + dynTile.collider.y;
    collidedRect.w = dynTile.collider.w;
    collidedRect.h = dynTile.collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;
      collidedWithDynTile = true;

      // Resolve along shallowest axis (same logic as static tiles).
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

      // Recalculate player collider after position correction.
      playerCollider.x = pos.x + collider.x;
      playerCollider.y = pos.y + collider.y;

      // Check if standing on top of the moving platform.
      if (playerCollider.y + playerCollider.h <= collidedRect.y) {
        foundGround = true;
      }

      // Carry the player horizontally with the platform's movement.
      pos.x += dynTile.vel.x * dt;
      // Only one dynTile collision per frame to avoid conflicting pushes.
      break;
    }
  }

  // --- Squish detection (dynTile + staticTile sandwich) ---
  // If the player is touching both a moving platform and a static tile
  // simultaneously, they are being crushed. The player dies unless they
  // are on top of the static tile (foundGround with it).
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

  // --- Coin overlap ---
  // Simple overlap check: if the player touches a coin, remove it and
  // increment the counter. Uses swap-and-pop (O(1) removal) by replacing
  // the current coin with the last one and shrinking the vector.
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

  // --- Slime overlap ---
  // Two outcomes depending on approach direction:
  //   1. Stomp (player coming from above): remove the slime, increment kill
  //      counter, and bounce the player upward (velocity stays positive so
  //      the player rises slightly).
  //   2. Contact from the side or below: player dies.
  for (size_t k = 0; k < slimes.size();) {
    collidedRect.x = slimes[k].pos.x + slimes[k].collider.x;
    collidedRect.y = slimes[k].pos.y + slimes[k].collider.y;
    collidedRect.w = slimes[k].collider.w;
    collidedRect.h = slimes[k].collider.h;

    if (SDL_GetRectIntersectionFloat(&playerCollider, &collidedRect,
                                     &intersectionRect)) {
      collided = true;

      // Stomp check: player must be falling (vel.y > 0) and the bottom of
      // the player collider must be near the top of the slime's collision
      // box (within 5 pixels). The 5-pixel tolerance makes stomps feel
      // more forgiving.
      if (vel.y > 0 &&
          (playerCollider.y + playerCollider.h - intersectionRect.h) <=
              slimes[k].pos.y + slimes[k].collider.y + 5.0f) {
        slimes[k] = slimes.back();
        slimes.pop_back();
        slainSlimes++;
        // Skip the k++ so the next iteration checks the swapped-in slime.
        continue;
      }
      // Not a stomp → player takes damage.
      currAnim = PlayerAnim::death;
    }
    k++;
  }

  // --- Update grounded state ---
  // If the player's bottom overlapped any tile top this frame, they are
  // grounded. Transition to the run animation on landing (overrides any
  // airborne animation like jump or falling).
  if (currAnim != PlayerAnim::death && grounded != foundGround) {
    grounded = foundGround;
    if (foundGround) {
      currAnim = PlayerAnim::run;
    }
  }
}
