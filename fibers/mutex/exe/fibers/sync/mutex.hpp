#pragma once

#include <exe/fibers/sync/futex.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

class Mutex {
 public:
  Mutex() : state_(0), counter_(0), futex_(FutexLike<uint32_t>(state_)) {
  }

  void Lock() {
    counter_.fetch_add(1);
    while (state_.exchange(1) != 0) {
      futex_.ParkIfEqual(1);
    }
  }

  void Unlock() {
    state_.store(0);
    if (counter_.fetch_sub(1) != 1) {
      futex_.WakeOne();
    }
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<uint32_t> state_;
  twist::stdlike::atomic<uint32_t> counter_;

  FutexLike<uint32_t> futex_;
};

}  // namespace exe::fibers
