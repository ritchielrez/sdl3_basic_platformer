#pragma once
#include <SDL3/SDL.h>

#include <cassert>
#include <glm/glm.hpp>
#include <vector>

#include "Timer.h"
#include "glm/fwd.hpp"

// Represents a sprite animation: a sequence of frames displayed over time.
// Handles frame indexing from elapsed time so the animation plays at a
// consistent speed regardless of frame rate.
class Frames {
  // Internal looping timer. Its total length is `len * frameCount` so it
  // cycles through all frames once per full loop.
  Timer timer;
  // Number of frames in this animation sequence. 1 = static/single-frame
  // sprite.
  size_t frameCount;
  // Dimensions (in pixels) of a single frame in the sprite sheet.
  uint16_t frameWidth, frameHeight;
  // Texture-space (UV) origin of each frame within the sprite sheet. Indexed
  // by frameIdx().
  std::vector<glm::vec2> texCoords;

 public:
  // Whether this animation loops. Non-looping animations stay on the last
  // frame after timeout instead of wrapping back to the first.
  bool loop = true;

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
  // for `len` seconds. Accepts rvalue vector of tex coords. A rvalue represents
  // a temporary object.
  Frames(int frameCount, float len, std::vector<glm::vec2> &&texCoords,
         uint16_t frameWidth, uint16_t frameHeight)
      : timer(len * static_cast<float>(frameCount)),
        frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(texCoords) {}
  // Same as above but accepts an lvalue reference vector. A lvalue represents
  // a object stored in RAM for significant amount of time.
  Frames(int frameCount, float len, std::vector<glm::vec2> &texCoords,
         uint16_t frameWidth, uint16_t frameHeight)
      : timer(len * static_cast<float>(frameCount)),
        frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(texCoords) {}

  void reset() { timer.reset(); }

  // Compute the current frame index by mapping the timer's progress through
  // one full cycle onto the frame range [0, frameCount). Single-frame
  // animations always return 0. Non-looping animations clamp to the last
  // frame once the timer has timed out.
  [[nodiscard]] int frameIdx() const {
    assert(frameCount != 0);
    if (frameCount == 1) return 0;
    if (!loop && timer.isTimeOut()) return static_cast<int>(frameCount) - 1;
    return static_cast<int>(timer.getTime() / timer.getLen() *
                            static_cast<float>(frameCount));
  }
  // Bunch of helper methods
  [[nodiscard]] bool isTimeOut() const { return timer.isTimeOut(); }
  [[nodiscard]] bool isStarted() const { return timer.isStarted(); }
  [[nodiscard]] float getLen() const { return timer.getLen(); }
  [[nodiscard]] uint16_t getFrameWidth() const { return frameWidth; }
  [[nodiscard]] uint16_t getFrameHeight() const { return frameHeight; }
  // Returns the UV origin of the current frame in the sprite sheet.
  [[nodiscard]] glm::vec2 getTexCoord() const {
    return texCoords.at(frameIdx());
  }

  // Step through the animation timer by `dt` (the time it takes each frame to process).
  void step(float dt) { timer.step(dt); }
};
