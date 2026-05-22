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

  bool isTimeOut() const { return timeout; }
  bool isStarted() const { return started; }
  float getTime() const { return time; }
  float getLen() const { return len; }
  void reset() {
    time = 0;
    timeout = false;
  }
};
