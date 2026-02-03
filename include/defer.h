// "Inspired" by <https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/>
#pragma once

namespace Private {
template <typename F> struct Defer {
  F f;
  Defer(F f) : f(f) {}
  ~Defer() { f(); }
};
} // namespace Private

template <typename F> static inline Private::Defer<F> defer_func(F f) {
  return Private::Defer<F>{f};
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) auto DEFER_3(_defer_) = defer_func([&]() { code; })
