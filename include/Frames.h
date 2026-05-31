#pragma once
#include <SDL3/SDL.h>

#include <cassert>
#include <glm/glm.hpp>
#include <vector>

#include "Timer.h"

// Represents a sprite animation: a sequence of frames displayed over time.
// Handles frame indexing from elapsed time so the animation plays at a
// consistent speed regardless of frame rate.
class Frames {
  // Internal looping timer. Its total length is `len * frameCount` so it
  // cycles through all frames once per full loop.
  Timer timer;
  // Number of frames in this animation sequence. 1 = static/single-frame sprite.
  size_t frameCount;
  // Dimensions (in pixels) of a single frame in the sprite sheet.
  uint16_t frameWidth, frameHeight;
  // Texture-space (UV) origin of each frame within the sprite sheet. Indexed
  // by frameIdx().
  std::vector<glm::vec2> texCoords;

 public:
  // Default constructor — no frames, length 0, no timer. Used as placeholder.
  Frames()
      : timer(0), frameCount(0), frameWidth(0), frameHeight(0), texCoords() {}
  // Single-frame (static) sprite: one texCoord, no animation.
  Frames(glm::vec2 &&texCoord, uint16_t frameWidth, uint16_t frameHeight)
      : timer(0),
        frameCount(1),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(1, texCoord) {}
  // Multi-frame animation: `frameCount` evenly-spaced frames, each displayed
  // for `len` seconds. Accepts rvalue vector of tex coords.
  Frames(int frameCount, float len, std::vector<glm::vec2> &&texCoords,
         uint16_t frameWidth, uint16_t frameHeight)
      : timer(len * frameCount),
        frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(texCoords) {}
  // Same as above but accepts an lvalue reference vector.
  Frames(int frameCount, float len, std::vector<glm::vec2> &texCoords,
         uint16_t frameWidth, uint16_t frameHeight)
      : timer(len * frameCount),
        frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(texCoords) {}

  // Compute the current frame index by mapping the timer's progress through
  // one full cycle onto the frame range [0, frameCount). Single-frame
  // animations always return 0.
  [[nodiscard]] int frameIdx() const {
    assert(frameCount != 0);
    return frameCount != 1
               ? static_cast<int>(timer.getTime() / timer.getLen() * frameCount)
               : 0;
  }
  [[nodiscard]] bool isTimeOut() const { return timer.isTimeOut(); }
  [[nodiscard]] bool isStarted() const { return timer.isStarted(); }
  [[nodiscard]] float getLen() const { return timer.getLen(); }
  [[nodiscard]] uint16_t getFrameWidth() const { return frameWidth; }
  [[nodiscard]] uint16_t getFrameHeight() const { return frameHeight; }
  // Returns the UV origin of the current frame in the sprite sheet.
  [[nodiscard]] glm::vec2 getTexCoord() const {
    return texCoords.at(frameIdx());
  }

  void step(float dt) { timer.step(dt); }
};
