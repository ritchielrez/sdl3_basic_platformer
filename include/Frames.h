#pragma once
#include <SDL3/SDL.h>

#include <cassert>
#include <glm/glm.hpp>
#include <vector>

#include "timer.h"

class Frames {
  Timer timer;
  size_t frameCount;
  std::vector<glm::vec2> texCoords;
  float frameWidth, frameHeight;

 public:
  Frames()
      : timer(0), frameCount(0), frameWidth(0), frameHeight(0), texCoords() {}
  Frames(glm::vec2 &&texCoord, float frameWidth, float frameHeight)
      : timer(0),
        frameCount(1),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(1, texCoord) {}
  Frames(int frameCount, float len, std::vector<glm::vec2> &&texCoords,
         float frameWidth, float frameHeight)
      : timer(len),
        frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(texCoords) {}
  Frames(int frameCount, float len, std::vector<glm::vec2> &texCoords,
         float frameWidth, float frameHeight)
      : timer(len),
        frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        texCoords(texCoords) {}

  int frameIdx() const {
    assert(frameCount != 0);
    return frameCount != 1
               ? static_cast<int>(timer.getTime() / timer.getLen() * frameCount)
               : 0;
  }
  float getLen() const { return timer.getLen(); }
  float getFrameWidth() const { return frameWidth; }
  float getFrameHeight() const { return frameWidth; }
  glm::vec2 getTexCoord() const { return texCoords.at(frameIdx()); }

  void step(float dt) { timer.step(dt); }
};
