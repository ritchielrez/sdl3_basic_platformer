#pragma once
#include <SDL3/SDL.h>

#include <cassert>
#include <glm/glm.hpp>
#include <vector>

#include "Timer.h"

class Frames {
  Timer timer;
  size_t frameCount;
  uint16_t frameWidth, frameHeight;
  std::vector<glm::vec2> texCoords;

 public:
  Frames()
      : timer(0), frameCount(0), frameWidth(0), frameHeight(0), texCoords() {}
  Frames(glm::vec2 &&texCoord, uint16_t frameWidth, uint16_t frameHeight)
      : timer(0),
        frameCount(1),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(1, texCoord) {}
  Frames(int frameCount, float len, std::vector<glm::vec2> &&texCoords,
         uint16_t frameWidth, uint16_t frameHeight)
      : timer(len * frameCount),
        frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(texCoords) {}
  Frames(int frameCount, float len, std::vector<glm::vec2> &texCoords,
         uint16_t frameWidth, uint16_t frameHeight)
      : timer(len * frameCount),
        frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(texCoords) {}

  [[nodiscard]] int frameIdx() const {
    assert(frameCount != 0);
    return frameCount != 1
               ? static_cast<int>(timer.getTime() / timer.getLen() * frameCount)
               : 0;
  }
  [[nodiscard]] float getLen() const { return timer.getLen(); }
  [[nodiscard]] uint16_t getFrameWidth() const { return frameWidth; }
  [[nodiscard]] uint16_t getFrameHeight() const { return frameHeight; }
  [[nodiscard]] glm::vec2 getTexCoord() const {
    return texCoords.at(frameIdx());
  }

  void step(float dt) { timer.step(dt); }
};
