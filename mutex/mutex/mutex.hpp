#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    counter_.fetch_add(1);
    while (state_.exchange(1) != 0) {
      state_.FutexWait(1);
    }
  }

  void Unlock() {
    state_.store(0);
    if (counter_.fetch_sub(1) != 1) {
      state_.FutexWakeOne();
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> state_{0};
  twist::stdlike::atomic<uint32_t> counter_{0};
};

}  // namespace stdlike
