#pragma once

#include <cassert>
#include <cmath>

class Timer {
  float len, time;
  bool timeout, started;

 public:
  Timer(float len) : len(len), time(0), timeout(false), started(false) {}

  void step(float dt) {
    assert(len != 0);
    started = true;
    time += dt;
    if (time >= len) {
      timeout = true;
      time = fmod(time, len);
    }
  }

  [[nodiscard]] bool isTimeOut() const { return timeout; }
  [[nodiscard]] bool isStarted() const { return started; }
  [[nodiscard]] float getTime() const { return time; }
  [[nodiscard]] float getLen() const { return len; }

  void reset() {
    time = 0;
    timeout = false;
  }
};
