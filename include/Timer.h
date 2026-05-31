#pragma once

#include <cassert>
#include <cmath>

// Frame-rate-independent timer used to drive game mechanics like cooldowns,
// animation durations, and movement grace periods (coyote time, jump buffering).
// The timer wraps around via fmod on expiry, enabling looping behavior for
// animations without manual reset.
class Timer {
  // Total duration of the timer in seconds.
  float len;
  // Accumulated elapsed time since last reset, modulo len.
  float time;
  // Set to true once the timer has completed a full cycle.
  bool timeout;
  // Set to true on the first step() call. Distinguishes "never started" from
  // "finished a cycle".
  bool started;

 public:
  Timer(float len) : len(len), time(0), timeout(false), started(false) {}

  // Advance the timer by dt seconds. Once `time >= len`, the timeout flag is
  // raised and time wraps around so the timer can loop (useful for repeating
  // animation frames or periodic behavior).
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

  // Reset all state back to initial (not started, not timed out, time = 0).
  // Used when reusing a timer for a new mechanic cycle (e.g. re-caching a
  // dash after cooldown).
  void reset() {
    time = 0;
    timeout = false;
    started = false;
  }
};
