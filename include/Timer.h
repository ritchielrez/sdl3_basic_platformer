#pragma once

#include <cmath>

class Timer {
  float len, time;
  bool timeout;

 public:
  Timer(float len) : len(len), time(0), timeout(false) {}

  void step(float dt) {
    assert(len != 0);
    time += dt;
    if (time >= len) {
      timeout = true;
      time = fmod(time, len);
    }
  }

  bool isTimeOut() const { return timeout; }
  float getTime() const { return time; }
  float getLen() const { return len; }
  void reset() { time = 0; }
};
