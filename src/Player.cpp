#include "Player.h"

#include "SDLState.h"

void Player::update(const SDLState& sdlState, float dt) {
  float currDir = 0;
  if (sdlState.keys[SDL_SCANCODE_A]) {
    currDir -= 1;
  }
  if (sdlState.keys[SDL_SCANCODE_D]) {
    currDir += 1;
  }
  if (currDir) {
    dir = currDir;
  }

  switch (currAnim) {
    case PlayerAnim::idle: {
      if (currDir) {
        currAnim = PlayerAnim::run;
      } else {
        if (vel.x) {
          const float factor = vel.x > 0 ? -1.5f : 1.5f;
          float amount = factor * accel.x * dt;

          if (glm::abs(vel.x) < glm::abs(amount)) {
            vel.x = 0;
          } else {
            vel.x += amount;
          }
        }
      }
      break;
    }
    case PlayerAnim::run: {
      if (!currDir) {
        currAnim = PlayerAnim::idle;
      }
      break;
    }
  }

  vel += currDir * accel * dt;
  if (glm::abs(vel.x) > maxSpeed.x) {
    vel.x = currDir * maxSpeed.x;
  }
  pos += vel * dt;
}

void collision(float dt) {}
